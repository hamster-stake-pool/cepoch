/*
 * tests/test_json_output.c - validates JSON schema of --output json
 *
 * Runs ./cepoch --output json and checks that the output is well-formed JSON
 * containing every expected key with the correct value type (number or string).
 *
 * Returns 0 on success, 1 on any failure.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int checks   = 0;

#define CHECK(label, cond)                                      \
    do {                                                        \
        checks++;                                               \
        if (!(cond)) {                                          \
            fprintf(stderr, "FAIL  %s\n", (label));            \
            failures++;                                         \
        }                                                       \
    } while (0)

/*
 * Return a pointer to the value part of "key": ... in buf,
 * skipping any leading whitespace after the colon.
 * Returns NULL if the key is not found.
 */
static const char *find_value(const char *buf, const char *key)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    const char *p = strstr(buf, pattern);
    if (!p)
        return NULL;
    p += strlen(pattern);
    while (*p == ' ' || *p == '\t')
        p++;
    return p;
}

static void check_number(const char *buf, const char *key)
{
    char label[80];
    snprintf(label, sizeof(label), "key \"%s\" present and numeric", key);
    const char *v = find_value(buf, key);
    if (!v) {
        fprintf(stderr, "FAIL  key \"%s\" not found\n", key);
        failures++;
        checks++;
        return;
    }
    CHECK(label, (*v >= '0' && *v <= '9') || *v == '-');
}

static void check_string(const char *buf, const char *key)
{
    char label[80];
    snprintf(label, sizeof(label), "key \"%s\" present and a string", key);
    const char *v = find_value(buf, key);
    if (!v) {
        fprintf(stderr, "FAIL  key \"%s\" not found\n", key);
        failures++;
        checks++;
        return;
    }
    CHECK(label, *v == '"');
}

int main(void)
{
    FILE *fp = popen("./cepoch --output json", "r");
    if (!fp) {
        fprintf(stderr, "FAIL  could not run ./cepoch --output json\n");
        return 1;
    }

    char buf[4096];
    size_t len = fread(buf, 1, sizeof(buf) - 1, fp);
    buf[len] = '\0';
    int rc = pclose(fp);

    /* --- structural checks ------------------------------------------ */
    CHECK("exit status 0",         rc == 0);
    CHECK("output starts with '{'", len > 0 && buf[0] == '{');

    /* find last non-whitespace character */
    size_t last = len;
    while (last > 0 &&
           (buf[last-1] == '\n' || buf[last-1] == '\r' || buf[last-1] == ' '))
        last--;
    CHECK("output ends with '}'", last > 0 && buf[last-1] == '}');

    /* --- numeric fields --------------------------------------------- */
    check_number(buf, "epoch");
    check_number(buf, "absolute_slot");
    check_number(buf, "epoch_slot");
    check_number(buf, "epoch_slots_total");
    check_number(buf, "time_left_seconds");
    check_number(buf, "epoch_start_unix");
    check_number(buf, "epoch_end_unix");

    /* --- string fields ---------------------------------------------- */
    check_string(buf, "time_left");
    check_string(buf, "epoch_start_utc");
    check_string(buf, "epoch_end_utc");
    check_string(buf, "local_time");
    check_string(buf, "utc_time");

    if (failures == 0)
        printf("All %d checks passed.\n", checks);
    else
        printf("%d/%d check(s) failed.\n", failures, checks);

    return failures > 0 ? 1 : 0;
}
