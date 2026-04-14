/*
 * tests/test_networks.c — unit tests for Pre-Production and Preview testnets
 *
 * Each test group calls cardano_set_network() before running its checks and
 * restores CARDANO_MAINNET at the end so test groups are fully independent.
 *
 * Returns 0 on success, 1 on any failure.
 *
 * Expected constants used below:
 *
 *  PREPROD
 *    Byron genesis:       1648771200  (2022-04-01 00:00:00 UTC)
 *    Byron slot duration: 20 s
 *    Byron epoch slots:   21600
 *    Shelley start epoch: 4
 *    Shelley start slot:  86400  (= 4 * 21600)
 *    Shelley start time:  1650499200  (2022-04-21 00:00:00 UTC)
 *    Shelley epoch slots: 432000
 *
 *  PREVIEW
 *    Shelley genesis:     1655683200  (2022-06-20 00:00:00 UTC)
 *    Shelley start epoch: 4  (epoch counter begins at 4, not 0)
 *    Shelley start slot:  0
 *    Shelley epoch slots: 432000
 */
#include "../src/cardano.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;
static int checks   = 0;

#define CHECK_EQ(label, got, expected)                                      \
    do {                                                                     \
        checks++;                                                            \
        if ((uint64_t)(got) != (uint64_t)(expected)) {                       \
            fprintf(stderr,                                                  \
                "FAIL  %s: got %" PRIu64 ", expected %" PRIu64 "\n",        \
                (label), (uint64_t)(got), (uint64_t)(expected));             \
            failures++;                                                      \
        }                                                                    \
    } while (0)

#define CHECK_STR(label, got, expected)                                      \
    do {                                                                      \
        checks++;                                                             \
        if (strcmp((got), (expected)) != 0) {                                 \
            fprintf(stderr, "FAIL  %s: got \"%s\", expected \"%s\"\n",       \
                (label), (got), (expected));                                  \
            failures++;                                                       \
        }                                                                     \
    } while (0)

/* =========================================================================
 * Pre-Production testnet (--testnet-magic 1)
 * ========================================================================= */

#define PREPROD_BYRON_GENESIS      UINT64_C(1648771200)
#define PREPROD_BYRON_SLOT_DUR     UINT64_C(20)
#define PREPROD_BYRON_EPOCH_SLOTS  UINT64_C(21600)
#define PREPROD_SHELLEY_EPOCH      UINT64_C(4)
#define PREPROD_SHELLEY_SLOT       UINT64_C(86400)   /* 4 * 21600 */
#define PREPROD_SHELLEY_TIME       UINT64_C(1650499200)
#define PREPROD_SHELLEY_SLOTS      UINT64_C(432000)

/* -- Byron era ------------------------------------------------------------ */

static void test_preprod_byron_genesis(void)
{
    cardano_info_t info = cardano_info_for_slot(0);
    CHECK_EQ("preprod_byron_genesis.epoch",           info.epoch,          0);
    CHECK_EQ("preprod_byron_genesis.epoch_slot",      info.epoch_slot,     0);
    CHECK_EQ("preprod_byron_genesis.abs_slot",        info.absolute_slot,  0);
    CHECK_EQ("preprod_byron_genesis.epoch_slots_total",
             info.epoch_slots_total, PREPROD_BYRON_EPOCH_SLOTS);
    CHECK_EQ("preprod_byron_genesis.start_time",
             (uint64_t)info.epoch_start_time, PREPROD_BYRON_GENESIS);
    CHECK_EQ("preprod_byron_genesis.end_time",
             (uint64_t)info.epoch_end_time,
             PREPROD_BYRON_GENESIS + PREPROD_BYRON_EPOCH_SLOTS * PREPROD_BYRON_SLOT_DUR);
}

static void test_preprod_byron_epoch0_last_slot(void)
{
    uint64_t last = PREPROD_BYRON_EPOCH_SLOTS - 1; /* 21599 */
    cardano_info_t info = cardano_info_for_slot(last);
    CHECK_EQ("preprod_byron0_last.epoch",      info.epoch,         0);
    CHECK_EQ("preprod_byron0_last.epoch_slot", info.epoch_slot,    last);
    CHECK_EQ("preprod_byron0_last.abs_slot",   info.absolute_slot, last);
    CHECK_EQ("preprod_byron0_last.epoch_slots_total",
             info.epoch_slots_total, PREPROD_BYRON_EPOCH_SLOTS);
}

static void test_preprod_byron_epoch1(void)
{
    cardano_info_t info = cardano_info_for_epoch(1);
    CHECK_EQ("preprod_byron1.epoch",      info.epoch,         1);
    CHECK_EQ("preprod_byron1.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preprod_byron1.abs_slot",   info.absolute_slot, PREPROD_BYRON_EPOCH_SLOTS);
    CHECK_EQ("preprod_byron1.start_time",
             (uint64_t)info.epoch_start_time,
             PREPROD_BYRON_GENESIS + PREPROD_BYRON_EPOCH_SLOTS * PREPROD_BYRON_SLOT_DUR);
}

static void test_preprod_byron_last_epoch(void)
{
    /* Epoch 3 is the last Byron epoch on preprod.
     * First slot: 3 * 21600 = 64800.
     * Last slot:  64800 + 21599 = 86399  (one before Shelley). */
    uint64_t epoch3_start_slot = 3 * PREPROD_BYRON_EPOCH_SLOTS;

    cardano_info_t info = cardano_info_for_epoch(3);
    CHECK_EQ("preprod_byron3.epoch",      info.epoch,         3);
    CHECK_EQ("preprod_byron3.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preprod_byron3.abs_slot",   info.absolute_slot, epoch3_start_slot);

    /* Last slot of epoch 3 */
    info = cardano_info_for_slot(epoch3_start_slot + PREPROD_BYRON_EPOCH_SLOTS - 1);
    CHECK_EQ("preprod_byron3_last.epoch",      info.epoch,     3);
    CHECK_EQ("preprod_byron3_last.epoch_slot", info.epoch_slot,
             PREPROD_BYRON_EPOCH_SLOTS - 1);

    /* Slot 86399: still Byron (one before Shelley start) */
    info = cardano_info_for_slot(PREPROD_SHELLEY_SLOT - 1);
    CHECK_EQ("preprod_pre_shelley.epoch",    info.epoch,         3);
    CHECK_EQ("preprod_pre_shelley.abs_slot", info.absolute_slot, PREPROD_SHELLEY_SLOT - 1);
}

/* -- Era boundary --------------------------------------------------------- */

static void test_preprod_era_boundary(void)
{
    /* Slot 86399 -> Byron epoch 3 */
    cardano_info_t before = cardano_info_for_slot(PREPROD_SHELLEY_SLOT - 1);
    CHECK_EQ("preprod_boundary_before.epoch", before.epoch, 3);

    /* Slot 86400 -> Shelley epoch 4 */
    cardano_info_t after = cardano_info_for_slot(PREPROD_SHELLEY_SLOT);
    CHECK_EQ("preprod_boundary_after.epoch",      after.epoch,         PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_boundary_after.epoch_slot", after.epoch_slot,    0);
    CHECK_EQ("preprod_boundary_after.abs_slot",   after.absolute_slot, PREPROD_SHELLEY_SLOT);
    CHECK_EQ("preprod_boundary_after.start_time",
             (uint64_t)after.epoch_start_time, PREPROD_SHELLEY_TIME);
}

/* -- Shelley era ---------------------------------------------------------- */

static void test_preprod_shelley_start(void)
{
    cardano_info_t info = cardano_info_for_slot(PREPROD_SHELLEY_SLOT);
    CHECK_EQ("preprod_shelley_start.epoch",            info.epoch,         PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_shelley_start.epoch_slot",       info.epoch_slot,    0);
    CHECK_EQ("preprod_shelley_start.abs_slot",         info.absolute_slot, PREPROD_SHELLEY_SLOT);
    CHECK_EQ("preprod_shelley_start.epoch_slots_total",
             info.epoch_slots_total, PREPROD_SHELLEY_SLOTS);
    CHECK_EQ("preprod_shelley_start.start_time",
             (uint64_t)info.epoch_start_time, PREPROD_SHELLEY_TIME);
    CHECK_EQ("preprod_shelley_start.end_time",
             (uint64_t)info.epoch_end_time, PREPROD_SHELLEY_TIME + PREPROD_SHELLEY_SLOTS);
}

static void test_preprod_shelley_epoch4_for_epoch(void)
{
    cardano_info_t info = cardano_info_for_epoch(PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_epoch4.epoch",      info.epoch,         PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_epoch4.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preprod_epoch4.abs_slot",   info.absolute_slot, PREPROD_SHELLEY_SLOT);
}

static void test_preprod_shelley_epoch5(void)
{
    uint64_t expected_abs   = PREPROD_SHELLEY_SLOT + PREPROD_SHELLEY_SLOTS;
    uint64_t expected_start = PREPROD_SHELLEY_TIME + PREPROD_SHELLEY_SLOTS;

    cardano_info_t info = cardano_info_for_epoch(5);
    CHECK_EQ("preprod_epoch5.epoch",      info.epoch,         5);
    CHECK_EQ("preprod_epoch5.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preprod_epoch5.abs_slot",   info.absolute_slot, expected_abs);
    CHECK_EQ("preprod_epoch5.start_time", (uint64_t)info.epoch_start_time, expected_start);
}

static void test_preprod_shelley_epoch4_last_slot(void)
{
    uint64_t last = PREPROD_SHELLEY_SLOT + PREPROD_SHELLEY_SLOTS - 1;
    cardano_info_t info = cardano_info_for_slot(last);
    CHECK_EQ("preprod_epoch4_last.epoch",      info.epoch,     PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_epoch4_last.epoch_slot", info.epoch_slot, PREPROD_SHELLEY_SLOTS - 1);
    CHECK_EQ("preprod_epoch4_last.abs_slot",   info.absolute_slot, last);
}

/* -- Time roundtrips ------------------------------------------------------ */

static void test_preprod_time_roundtrip(void)
{
    /* Byron genesis time -> slot 0, epoch 0 */
    cardano_info_t info = cardano_info_for_time((time_t)PREPROD_BYRON_GENESIS);
    CHECK_EQ("preprod_time_byron_genesis.epoch",    info.epoch,         0);
    CHECK_EQ("preprod_time_byron_genesis.abs_slot", info.absolute_slot, 0);

    /* Pre-genesis: must clamp to slot 0 */
    info = cardano_info_for_time((time_t)(PREPROD_BYRON_GENESIS - 1));
    CHECK_EQ("preprod_time_pre_genesis.abs_slot", info.absolute_slot, 0);

    /* 100 Byron slots in (100 * 20 s) */
    time_t mid_byron = (time_t)(PREPROD_BYRON_GENESIS + 100 * PREPROD_BYRON_SLOT_DUR);
    info = cardano_info_for_time(mid_byron);
    CHECK_EQ("preprod_time_byron_mid.epoch",      info.epoch,          0);
    CHECK_EQ("preprod_time_byron_mid.abs_slot",   info.absolute_slot, 100);
    CHECK_EQ("preprod_time_byron_mid.epoch_slot", info.epoch_slot,    100);

    /* Shelley start time -> slot 86400, epoch 4, epoch_slot 0 */
    info = cardano_info_for_time((time_t)PREPROD_SHELLEY_TIME);
    CHECK_EQ("preprod_time_shelley_start.epoch",      info.epoch,     PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_time_shelley_start.epoch_slot", info.epoch_slot, 0);
    CHECK_EQ("preprod_time_shelley_start.abs_slot",   info.absolute_slot, PREPROD_SHELLEY_SLOT);

    /* 1000 seconds into Shelley -> epoch_slot 1000 */
    info = cardano_info_for_time((time_t)(PREPROD_SHELLEY_TIME + 1000));
    CHECK_EQ("preprod_time_shelley+1000.epoch",      info.epoch,      PREPROD_SHELLEY_EPOCH);
    CHECK_EQ("preprod_time_shelley+1000.epoch_slot", info.epoch_slot, 1000);
    CHECK_EQ("preprod_time_shelley+1000.abs_slot",
             info.absolute_slot, PREPROD_SHELLEY_SLOT + 1000);
}

/* -- epoch_end_time sanity ------------------------------------------------ */

static void test_preprod_epoch_end_time(void)
{
    /* Byron: end of epoch 3 == start of epoch 4 */
    cardano_info_t e3 = cardano_info_for_epoch(3);
    cardano_info_t e4 = cardano_info_for_epoch(4);
    CHECK_EQ("preprod_end3_eq_start4",
             (uint64_t)e3.epoch_end_time, (uint64_t)e4.epoch_start_time);

    /* Shelley: end of epoch 4 == start of epoch 5 */
    cardano_info_t e5 = cardano_info_for_epoch(5);
    CHECK_EQ("preprod_end4_eq_start5",
             (uint64_t)e4.epoch_end_time, (uint64_t)e5.epoch_start_time);

    CHECK_EQ("preprod_end3_gt_start3",
             (uint64_t)e3.epoch_end_time > (uint64_t)e3.epoch_start_time, 1);
    CHECK_EQ("preprod_end4_gt_start4",
             (uint64_t)e4.epoch_end_time > (uint64_t)e4.epoch_start_time, 1);
}

/* -- Epoch/slot consistency ----------------------------------------------- */

static void test_preprod_epoch_slot_consistency(void)
{
    uint64_t epochs[] = {0, 1, 2, 3, 4, 5, 10, 100};
    size_t n = sizeof(epochs) / sizeof(epochs[0]);
    char label[64];

    for (size_t i = 0; i < n; i++) {
        cardano_info_t fe = cardano_info_for_epoch(epochs[i]);
        cardano_info_t fs = cardano_info_for_slot(fe.absolute_slot);

        snprintf(label, sizeof(label),
                 "preprod_consistency[%" PRIu64 "].epoch", epochs[i]);
        CHECK_EQ(label, fs.epoch, fe.epoch);

        snprintf(label, sizeof(label),
                 "preprod_consistency[%" PRIu64 "].start_time", epochs[i]);
        CHECK_EQ(label,
                 (uint64_t)fs.epoch_start_time, (uint64_t)fe.epoch_start_time);
    }
}

/* -- Network name --------------------------------------------------------- */

static void test_preprod_network_name(void)
{
    CHECK_STR("preprod_network_name", cardano_network_name(), "preprod");
}

/* =========================================================================
 * Preview testnet (--testnet-magic 2)
 * ========================================================================= */

#define PREVIEW_GENESIS       UINT64_C(1655683200)  /* 2022-06-20 00:00:00 UTC */
#define PREVIEW_START_EPOCH   UINT64_C(4)            /* epoch counter starts at 4 */
#define PREVIEW_EPOCH_SLOTS   UINT64_C(432000)

/* First valid epoch on Preview is 4 (slot 0).
 * epoch = PREVIEW_START_EPOCH + absolute_slot / PREVIEW_EPOCH_SLOTS */

static void test_preview_genesis_slot(void)
{
    /* Slot 0 is the genesis slot; the first epoch is epoch 4. */
    cardano_info_t info = cardano_info_for_slot(0);
    CHECK_EQ("preview_genesis.epoch",            info.epoch,          PREVIEW_START_EPOCH);
    CHECK_EQ("preview_genesis.epoch_slot",       info.epoch_slot,     0);
    CHECK_EQ("preview_genesis.abs_slot",         info.absolute_slot,  0);
    CHECK_EQ("preview_genesis.epoch_slots_total",
             info.epoch_slots_total, PREVIEW_EPOCH_SLOTS);
    CHECK_EQ("preview_genesis.start_time",
             (uint64_t)info.epoch_start_time, PREVIEW_GENESIS);
    CHECK_EQ("preview_genesis.end_time",
             (uint64_t)info.epoch_end_time, PREVIEW_GENESIS + PREVIEW_EPOCH_SLOTS);
}

static void test_preview_epoch4_last_slot(void)
{
    uint64_t last = PREVIEW_EPOCH_SLOTS - 1; /* 431999 */
    cardano_info_t info = cardano_info_for_slot(last);
    CHECK_EQ("preview_epoch4_last.epoch",      info.epoch,         PREVIEW_START_EPOCH);
    CHECK_EQ("preview_epoch4_last.epoch_slot", info.epoch_slot,    last);
    CHECK_EQ("preview_epoch4_last.abs_slot",   info.absolute_slot, last);
}

static void test_preview_epoch5(void)
{
    /* First slot of epoch 5 */
    cardano_info_t info_slot  = cardano_info_for_slot(PREVIEW_EPOCH_SLOTS);
    cardano_info_t info_epoch = cardano_info_for_epoch(PREVIEW_START_EPOCH + 1);

    CHECK_EQ("preview_epoch5_slot.epoch",      info_slot.epoch,         PREVIEW_START_EPOCH + 1);
    CHECK_EQ("preview_epoch5_slot.epoch_slot", info_slot.epoch_slot,    0);
    CHECK_EQ("preview_epoch5_slot.abs_slot",   info_slot.absolute_slot, PREVIEW_EPOCH_SLOTS);
    CHECK_EQ("preview_epoch5_slot.start_time",
             (uint64_t)info_slot.epoch_start_time,
             PREVIEW_GENESIS + PREVIEW_EPOCH_SLOTS);

    CHECK_EQ("preview_epoch5_for_epoch.epoch",
             info_epoch.epoch,         PREVIEW_START_EPOCH + 1);
    CHECK_EQ("preview_epoch5_for_epoch.epoch_slot", info_epoch.epoch_slot,    0);
    CHECK_EQ("preview_epoch5_for_epoch.abs_slot",
             info_epoch.absolute_slot, PREVIEW_EPOCH_SLOTS);
}

static void test_preview_far_epoch(void)
{
    /* Epoch 100 (shelley_epoch = 100 - 4 = 96) */
    uint64_t shelley_epoch  = UINT64_C(100) - PREVIEW_START_EPOCH;
    uint64_t expected_abs   = shelley_epoch * PREVIEW_EPOCH_SLOTS;
    uint64_t expected_start = PREVIEW_GENESIS + shelley_epoch * PREVIEW_EPOCH_SLOTS;

    cardano_info_t info = cardano_info_for_epoch(100);
    CHECK_EQ("preview_epoch100.epoch",      info.epoch,         100);
    CHECK_EQ("preview_epoch100.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preview_epoch100.abs_slot",   info.absolute_slot, expected_abs);
    CHECK_EQ("preview_epoch100.start_time",
             (uint64_t)info.epoch_start_time, expected_start);
}

static void test_preview_time_roundtrip(void)
{
    /* Genesis time -> slot 0, epoch 4, epoch_slot 0 */
    cardano_info_t info = cardano_info_for_time((time_t)PREVIEW_GENESIS);
    CHECK_EQ("preview_time_genesis.epoch",      info.epoch,         PREVIEW_START_EPOCH);
    CHECK_EQ("preview_time_genesis.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preview_time_genesis.abs_slot",   info.absolute_slot, 0);

    /* Pre-genesis: must clamp to slot 0 */
    info = cardano_info_for_time((time_t)(PREVIEW_GENESIS - 1));
    CHECK_EQ("preview_time_pre_genesis.abs_slot", info.absolute_slot, 0);

    /* 1000 seconds in -> slot 1000, epoch 4, epoch_slot 1000 */
    info = cardano_info_for_time((time_t)(PREVIEW_GENESIS + 1000));
    CHECK_EQ("preview_time+1000.epoch",      info.epoch,          PREVIEW_START_EPOCH);
    CHECK_EQ("preview_time+1000.epoch_slot", info.epoch_slot,    1000);
    CHECK_EQ("preview_time+1000.abs_slot",   info.absolute_slot, 1000);

    /* Exactly one epoch in -> slot 432000, epoch 5, epoch_slot 0 */
    info = cardano_info_for_time((time_t)(PREVIEW_GENESIS + PREVIEW_EPOCH_SLOTS));
    CHECK_EQ("preview_time_epoch5.epoch",      info.epoch,         PREVIEW_START_EPOCH + 1);
    CHECK_EQ("preview_time_epoch5.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("preview_time_epoch5.abs_slot",   info.absolute_slot, PREVIEW_EPOCH_SLOTS);
}

static void test_preview_epoch_end_time(void)
{
    cardano_info_t e4 = cardano_info_for_epoch(PREVIEW_START_EPOCH);
    cardano_info_t e5 = cardano_info_for_epoch(PREVIEW_START_EPOCH + 1);
    cardano_info_t e6 = cardano_info_for_epoch(PREVIEW_START_EPOCH + 2);

    CHECK_EQ("preview_end4_eq_start5",
             (uint64_t)e4.epoch_end_time, (uint64_t)e5.epoch_start_time);
    CHECK_EQ("preview_end5_eq_start6",
             (uint64_t)e5.epoch_end_time, (uint64_t)e6.epoch_start_time);
    CHECK_EQ("preview_end4_gt_start4",
             (uint64_t)e4.epoch_end_time > (uint64_t)e4.epoch_start_time, 1);
}

static void test_preview_epoch_slots_total(void)
{
    /* Preview has no Byron era, so epoch_slots_total is always 432000 */
    cardano_info_t e4  = cardano_info_for_epoch(PREVIEW_START_EPOCH);
    cardano_info_t e10 = cardano_info_for_epoch(10);
    CHECK_EQ("preview_epoch4_slots_total",  e4.epoch_slots_total,  PREVIEW_EPOCH_SLOTS);
    CHECK_EQ("preview_epoch10_slots_total", e10.epoch_slots_total, PREVIEW_EPOCH_SLOTS);
}

static void test_preview_epoch_slot_consistency(void)
{
    /* Use only epochs >= PREVIEW_START_EPOCH (4) since epochs 0-3 don't exist
     * on the Preview chain. */
    uint64_t epochs[] = {4, 5, 6, 10, 50, 100, 282};
    size_t n = sizeof(epochs) / sizeof(epochs[0]);
    char label[64];

    for (size_t i = 0; i < n; i++) {
        cardano_info_t fe = cardano_info_for_epoch(epochs[i]);
        cardano_info_t fs = cardano_info_for_slot(fe.absolute_slot);

        snprintf(label, sizeof(label),
                 "preview_consistency[%" PRIu64 "].epoch", epochs[i]);
        CHECK_EQ(label, fs.epoch, fe.epoch);

        snprintf(label, sizeof(label),
                 "preview_consistency[%" PRIu64 "].start_time", epochs[i]);
        CHECK_EQ(label,
                 (uint64_t)fs.epoch_start_time, (uint64_t)fe.epoch_start_time);
    }
}

static void test_preview_network_name(void)
{
    CHECK_STR("preview_network_name", cardano_network_name(), "preview");
}

/* =========================================================================
 * Cross-network switch test
 * ========================================================================= */

static void test_network_switch(void)
{
    /* Slot 0 on mainnet is Byron epoch 0, genesis 1506203091 */
    cardano_set_network(&CARDANO_MAINNET);
    cardano_info_t mn = cardano_info_for_slot(0);
    CHECK_EQ("switch_mainnet.epoch",      mn.epoch,         0);
    CHECK_EQ("switch_mainnet.start_time",
             (uint64_t)mn.epoch_start_time, UINT64_C(1506203091));
    CHECK_STR("switch_mainnet.name", cardano_network_name(), "mainnet");

    /* Same slot on preprod: different genesis */
    cardano_set_network(&CARDANO_PREPROD);
    cardano_info_t pp = cardano_info_for_slot(0);
    CHECK_EQ("switch_preprod.epoch",      pp.epoch,         0);
    CHECK_EQ("switch_preprod.start_time",
             (uint64_t)pp.epoch_start_time, PREPROD_BYRON_GENESIS);
    CHECK_STR("switch_preprod.name", cardano_network_name(), "preprod");

    /* Same slot on preview: different genesis, epoch starts at 4 */
    cardano_set_network(&CARDANO_PREVIEW);
    cardano_info_t pv = cardano_info_for_slot(0);
    CHECK_EQ("switch_preview.epoch",      pv.epoch,         PREVIEW_START_EPOCH);
    CHECK_EQ("switch_preview.start_time",
             (uint64_t)pv.epoch_start_time, PREVIEW_GENESIS);
    CHECK_STR("switch_preview.name", cardano_network_name(), "preview");
}

/* =========================================================================
 * main
 * ========================================================================= */

int main(void)
{
    /* ------------------------------------------------------------------ */
    printf("=== Pre-Production testnet ===\n");
    cardano_set_network(&CARDANO_PREPROD);

    test_preprod_network_name();
    test_preprod_byron_genesis();
    test_preprod_byron_epoch0_last_slot();
    test_preprod_byron_epoch1();
    test_preprod_byron_last_epoch();
    test_preprod_era_boundary();
    test_preprod_shelley_start();
    test_preprod_shelley_epoch4_for_epoch();
    test_preprod_shelley_epoch4_last_slot();
    test_preprod_shelley_epoch5();
    test_preprod_time_roundtrip();
    test_preprod_epoch_end_time();
    test_preprod_epoch_slot_consistency();

    /* ------------------------------------------------------------------ */
    printf("=== Preview testnet ===\n");
    cardano_set_network(&CARDANO_PREVIEW);

    test_preview_network_name();
    test_preview_genesis_slot();
    test_preview_epoch4_last_slot();
    test_preview_epoch5();
    test_preview_far_epoch();
    test_preview_time_roundtrip();
    test_preview_epoch_end_time();
    test_preview_epoch_slots_total();
    test_preview_epoch_slot_consistency();

    /* ------------------------------------------------------------------ */
    printf("=== Network switch ===\n");
    test_network_switch();

    /* Restore mainnet so the binary is in a defined state when it exits */
    cardano_set_network(&CARDANO_MAINNET);

    /* ------------------------------------------------------------------ */
    if (failures == 0)
        printf("All %d checks passed.\n", checks);
    else
        printf("%d/%d check(s) failed.\n", failures, checks);

    return failures > 0 ? 1 : 0;
}
