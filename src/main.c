/*
 * main.c - cepoch CLI
 *
 * Copyright (c) 2026 Neal Allen Morrison <nmorrison@magister-technicus.de>
 * Magister Technicus GmbH
 * SPDX-License-Identifier: MIT
 *
 * Displays Cardano epoch/slot information.
 *
 * Usage:
 *   cepoch                              current epoch info (mainnet)
 *   cepoch --mainnet                    explicitly select mainnet (default)
 *   cepoch --testnet-magic 1            Pre-Production testnet
 *   cepoch --testnet-magic 2            Preview testnet
 *   cepoch --epoch N                    info for epoch N
 *   cepoch --slot N                     info for absolute slot N
 *   cepoch --epoch-slot N               info for epoch-slot N in the current epoch
 *   cepoch --date YYYY-MM-DDTHH:MM:SSZ  info for a UTC timestamp
 */
#include "config.h"
#include "cardano.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

static void format_remaining(char *buf, size_t len, time_t seconds)
{
    if (seconds <= 0) {
        snprintf(buf, len, "elapsed");
        return;
    }
    uint64_t s     = (uint64_t)seconds;
    uint64_t days  = s / 86400;
    uint64_t hours = (s % 86400) / 3600;
    uint64_t mins  = (s % 3600) / 60;
    uint64_t secs  = s % 60;
    snprintf(buf, len, "%" PRIu64 "d %" PRIu64 "h %" PRIu64 "m %" PRIu64 "s",
             days, hours, mins, secs);
}

/*
 * Parse an ISO-8601 UTC date string of the form YYYY-MM-DDTHH:MM:SSZ
 * and return the corresponding Unix timestamp.
 * Returns -1 on parse error.
 */
static time_t parse_iso8601_utc(const char *s)
{
    int year, month, day, hour, min, sec;
    if (sscanf(s, "%d-%d-%dT%d:%d:%dZ",
               &year, &month, &day, &hour, &min, &sec) != 6) {
        return (time_t)-1;
    }
    if (month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || min < 0 || min > 59 ||
        sec < 0 || sec > 60) {
        return (time_t)-1;
    }

    /* Days in each month for a non-leap year */
    static const int dom[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

    /* Count days from 1970-01-01 */
    long days = 0;
    int y;
    for (y = 1970; y < year; y++) {
        days += 365;
        if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
            days++;
    }
    int leap = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
    int m;
    for (m = 1; m < month; m++) {
        days += dom[m - 1];
        if (m == 2 && leap) days++;
    }
    days += day - 1;

    return (time_t)(days * 86400L + hour * 3600L + min * 60L + sec);
}

/* -------------------------------------------------------------------------
 * Output
 * ---------------------------------------------------------------------- */

static int g_json_out = 0; /* set by --output json */

/*
 * Collect all display fields into a single helper to avoid repeating
 * the platform-specific time formatting in both output functions.
 */
typedef struct {
    char     remaining_str[32];
    int64_t  remaining_seconds;
    char     start_str[32];
    char     end_str[32];
    char     local_str[32];
    char     utc_str[32];
    time_t   now;
} display_fields_t;

static display_fields_t build_display_fields(const cardano_info_t *info)
{
    display_fields_t f;
    struct tm tm_tmp;

    f.now = time(NULL);
    f.remaining_seconds = (int64_t)(info->epoch_end_time - f.now);
    format_remaining(f.remaining_str, sizeof(f.remaining_str), f.remaining_seconds);

#ifdef HAVE_GMTIME_R
    gmtime_r(&info->epoch_start_time, &tm_tmp);
#else
    { struct tm *p = gmtime(&info->epoch_start_time); if (p) tm_tmp = *p; }
#endif
    strftime(f.start_str, sizeof(f.start_str), "%Y-%m-%d %H:%M:%S", &tm_tmp);

#ifdef HAVE_GMTIME_R
    gmtime_r(&info->epoch_end_time, &tm_tmp);
#else
    { struct tm *p = gmtime(&info->epoch_end_time); if (p) tm_tmp = *p; }
#endif
    strftime(f.end_str, sizeof(f.end_str), "%Y-%m-%d %H:%M:%S", &tm_tmp);

#ifdef HAVE_LOCALTIME_R
    localtime_r(&f.now, &tm_tmp);
#else
    { struct tm *p = localtime(&f.now); if (p) tm_tmp = *p; }
#endif
    strftime(f.local_str, sizeof(f.local_str), "%Y-%m-%d %H:%M:%S", &tm_tmp);

#ifdef HAVE_GMTIME_R
    gmtime_r(&f.now, &tm_tmp);
#else
    { struct tm *p = gmtime(&f.now); if (p) tm_tmp = *p; }
#endif
    strftime(f.utc_str, sizeof(f.utc_str), "%Y-%m-%d %H:%M:%S", &tm_tmp);

    return f;
}

static void print_info(const cardano_info_t *info)
{
    display_fields_t f = build_display_fields(info);

    printf("Network:       %s\n",                  cardano_network_name());
    printf("Epoch:         %" PRIu64 "\n",         info->epoch);
    printf("Absolute Slot: %" PRIu64 "\n",         info->absolute_slot);
    printf("Epoch Slot:    %" PRIu64 " / %" PRIu64 "\n",
           info->epoch_slot, info->epoch_slots_total);
    printf("Time left:     %s\n",                  f.remaining_str);
    printf("Epoch range:   %s UTC - %s UTC\n",     f.start_str, f.end_str);
    printf("Local time:    %s\n",                  f.local_str);
    printf("UTC time:      %s UTC\n",              f.utc_str);
}

static void print_info_json(const cardano_info_t *info)
{
    display_fields_t f = build_display_fields(info);

    printf("{\n");
    printf("  \"network\": \"%s\",\n",              cardano_network_name());
    printf("  \"epoch\": %" PRIu64 ",\n",            info->epoch);
    printf("  \"absolute_slot\": %" PRIu64 ",\n",    info->absolute_slot);
    printf("  \"epoch_slot\": %" PRIu64 ",\n",       info->epoch_slot);
    printf("  \"epoch_slots_total\": %" PRIu64 ",\n", info->epoch_slots_total);
    printf("  \"time_left_seconds\": %" PRId64 ",\n", (int64_t)f.remaining_seconds);
    printf("  \"time_left\": \"%s\",\n",              f.remaining_str);
    printf("  \"epoch_start_utc\": \"%s\",\n",        f.start_str);
    printf("  \"epoch_end_utc\": \"%s\",\n",          f.end_str);
    printf("  \"epoch_start_unix\": %lld,\n",         (long long)info->epoch_start_time);
    printf("  \"epoch_end_unix\": %lld,\n",           (long long)info->epoch_end_time);
    printf("  \"local_time\": \"%s\",\n",             f.local_str);
    printf("  \"utc_time\": \"%s\"\n",                f.utc_str);
    printf("}\n");
}

static void output_info(const cardano_info_t *info)
{
    if (g_json_out)
        print_info_json(info);
    else
        print_info(info);
}

/* -------------------------------------------------------------------------
 * Entry point
 * ---------------------------------------------------------------------- */

static void print_delegation_notice(FILE *out)
{
    fprintf(out,
        "---------------------------------------------------------------------\n"
        "If you find this tool useful, please consider delegating your ADA to:\n"
        "\n"
        "  Hamster Stake Pool  [HAMDA]\n"
        "\n"
        "Your delegation supports continued development and the Cardano network.\n"
        "---------------------------------------------------------------------\n");
}

static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [NETWORK] [OPTION]\n"
        "\n"
        "Network selection (default: --mainnet):\n"
        "  --mainnet                  Cardano mainnet (default)\n"
        "  --testnet-magic 1          Pre-Production testnet\n"
        "  --testnet-magic 2          Preview testnet\n"
        "\n"
        "Query options:\n"
        "  (no query option)          show current epoch info\n"
        "  --epoch N                  info for epoch N\n"
        "  --slot N                   info for absolute slot N\n"
        "  --epoch-slot N             info for epoch-slot N in the current epoch\n"
        "  --date YYYY-MM-DDTHH:MM:SSZ  info for a UTC date/time\n"
        "\n"
        "Output options:\n"
        "  --output json              output result as JSON\n"
        "\n"
        "Miscellaneous:\n"
        "  --version                  show version information\n"
        "  --help                     show this help\n"
        "\n",
        prog);
    print_delegation_notice(stderr);
}

int main(int argc, char *argv[])
{
    int i;

    /* ------------------------------------------------------------------
     * Pre-scan: process global flags that must be set before any queries.
     * ------------------------------------------------------------------ */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc && strcmp(argv[i + 1], "json") == 0)
                g_json_out = 1;

        } else if (strcmp(argv[i], "--mainnet") == 0) {
            cardano_set_network(&CARDANO_MAINNET);

        } else if (strcmp(argv[i], "--testnet-magic") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --testnet-magic requires a value\n");
                return 1;
            }
            char *endp;
            uint64_t magic = strtoull(argv[i + 1], &endp, 10);
            if (*endp != '\0') {
                fprintf(stderr, "error: invalid testnet magic '%s'\n", argv[i + 1]);
                return 1;
            }
            if (magic == 1) {
                cardano_set_network(&CARDANO_PREPROD);
            } else if (magic == 2) {
                cardano_set_network(&CARDANO_PREVIEW);
            } else {
                fprintf(stderr,
                    "error: unsupported testnet magic %" PRIu64
                    " (supported: 1 = preprod, 2 = preview)\n", magic);
                return 1;
            }
            ++i; /* skip the value token in this loop */

        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;

        } else if (strcmp(argv[i], "--version") == 0) {
            printf("%s %s\n", PACKAGE, VERSION);
            printf("License: MIT — see LICENSE file or <https://opensource.org/licenses/MIT>\n");
            printf("Copyright (c) 2026 Neal Allen Morrison <nmorrison@magister-technicus.de>\n");
            print_delegation_notice(stdout);
            return 0;
        }
    }

    /* ------------------------------------------------------------------
     * Main processing loop — execute query options.
     * ------------------------------------------------------------------ */
    int mode_selected = 0;

    for (i = 1; i < argc; i++) {
        const char *opt = argv[i];

        if (strcmp(opt, "--output") == 0) {
            /* already handled; consume value */
            ++i;
            continue;

        } else if (strcmp(opt, "--mainnet") == 0 ||
                   strcmp(opt, "--help")    == 0 ||
                   strcmp(opt, "--version") == 0) {
            /* already handled in pre-scan */
            continue;

        } else if (strcmp(opt, "--testnet-magic") == 0) {
            /* already handled in pre-scan; consume value */
            ++i;
            continue;

        } else if (strcmp(opt, "--epoch") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --epoch requires a value\n");
                return 1;
            }
            char *endp;
            uint64_t epoch = strtoull(argv[++i], &endp, 10);
            if (*endp != '\0') {
                fprintf(stderr, "error: invalid epoch '%s'\n", argv[i]);
                return 1;
            }
            cardano_info_t info = cardano_info_for_epoch(epoch);
            output_info(&info);
            mode_selected = 1;

        } else if (strcmp(opt, "--slot") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --slot requires a value\n");
                return 1;
            }
            char *endp;
            uint64_t slot = strtoull(argv[++i], &endp, 10);
            if (*endp != '\0') {
                fprintf(stderr, "error: invalid slot '%s'\n", argv[i]);
                return 1;
            }
            cardano_info_t info = cardano_info_for_slot(slot);
            output_info(&info);
            mode_selected = 1;

        } else if (strcmp(opt, "--epoch-slot") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --epoch-slot requires a value\n");
                return 1;
            }
            char *endp;
            uint64_t eslot = strtoull(argv[++i], &endp, 10);
            if (*endp != '\0') {
                fprintf(stderr, "error: invalid epoch-slot '%s'\n", argv[i]);
                return 1;
            }
            cardano_info_t cur = cardano_current_info();
            if (eslot >= cur.epoch_slots_total) {
                fprintf(stderr,
                    "error: epoch-slot %" PRIu64
                    " out of range (max %" PRIu64 ")\n",
                    eslot, cur.epoch_slots_total - 1);
                return 1;
            }
            cardano_info_t info = cardano_info_for_current_epoch_slot(eslot);
            output_info(&info);
            mode_selected = 1;

        } else if (strcmp(opt, "--date") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: --date requires a value\n");
                return 1;
            }
            const char *datestr = argv[++i];
            time_t t = parse_iso8601_utc(datestr);
            if (t == (time_t)-1) {
                fprintf(stderr,
                    "error: invalid date '%s', expected YYYY-MM-DDTHH:MM:SSZ\n",
                    datestr);
                return 1;
            }
            cardano_info_t info = cardano_info_for_time(t);
            output_info(&info);
            mode_selected = 1;

        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            usage(argv[0]);
            return 1;
        }
    }

    /* No query option given: show current epoch info */
    if (!mode_selected) {
        cardano_info_t info = cardano_current_info();
        output_info(&info);
    }

    return 0;
}
