/*
 * tests/test_cardano.c - unit tests for cardano.c
 *
 * Returns 0 on success, 1 on any failure.
 */
#include "../src/cardano.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static int failures = 0;
static int checks   = 0;

#define CHECK_EQ(label, got, expected)                                      \
    do {                                                                     \
        checks++;                                                            \
        if ((got) != (expected)) {                                           \
            fprintf(stderr,                                                  \
                "FAIL  %s: got %" PRIu64 ", expected %" PRIu64 "\n",        \
                (label), (uint64_t)(got), (uint64_t)(expected));             \
            failures++;                                                      \
        }                                                                    \
    } while (0)

/* ------------------------------------------------------------------ */
/* Byron era                                                            */
/* ------------------------------------------------------------------ */

static void test_byron_genesis(void)
{
    cardano_info_t info = cardano_info_for_slot(0);
    CHECK_EQ("byron_genesis.epoch",      info.epoch,          0);
    CHECK_EQ("byron_genesis.epoch_slot", info.epoch_slot,     0);
    CHECK_EQ("byron_genesis.abs_slot",   info.absolute_slot,  0);
    CHECK_EQ("byron_genesis.start_time", (uint64_t)info.epoch_start_time,
             CARDANO_BYRON_GENESIS_TIME);
    /* epoch end = genesis + 21600 * 20 = genesis + 432000 */
    CHECK_EQ("byron_genesis.end_time",   (uint64_t)info.epoch_end_time,
             CARDANO_BYRON_GENESIS_TIME + CARDANO_BYRON_EPOCH_SLOTS
                                        * CARDANO_BYRON_SLOT_DURATION);
}

static void test_byron_epoch0_last_slot(void)
{
    uint64_t last = CARDANO_BYRON_EPOCH_SLOTS - 1;  /* 21599 */
    cardano_info_t info = cardano_info_for_slot(last);
    CHECK_EQ("byron0_last.epoch",      info.epoch,          0);
    CHECK_EQ("byron0_last.epoch_slot", info.epoch_slot,     last);
    CHECK_EQ("byron0_last.abs_slot",   info.absolute_slot,  last);
}

static void test_byron_epoch1(void)
{
    cardano_info_t info = cardano_info_for_epoch(1);
    CHECK_EQ("byron1.epoch",      info.epoch,         1);
    CHECK_EQ("byron1.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("byron1.abs_slot",   info.absolute_slot, CARDANO_BYRON_EPOCH_SLOTS);
    CHECK_EQ("byron1.start_time", (uint64_t)info.epoch_start_time,
             CARDANO_BYRON_GENESIS_TIME
             + CARDANO_BYRON_EPOCH_SLOTS * CARDANO_BYRON_SLOT_DURATION);
}

static void test_byron_last_epoch(void)
{
    /* Epoch 207 is the last Byron epoch.
     * Its first absolute slot = 207 * 21600 = 4465200.
     * Its last absolute slot  = 4465200 + 21599 = 4486799.
     * Slot 4492799 is the very last slot before Shelley. */
    uint64_t epoch207_start = 207 * CARDANO_BYRON_EPOCH_SLOTS;

    cardano_info_t info = cardano_info_for_epoch(207);
    CHECK_EQ("byron207.epoch",      info.epoch,         207);
    CHECK_EQ("byron207.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("byron207.abs_slot",   info.absolute_slot, epoch207_start);

    /* last slot of epoch 207 */
    info = cardano_info_for_slot(epoch207_start + CARDANO_BYRON_EPOCH_SLOTS - 1);
    CHECK_EQ("byron207_last.epoch",      info.epoch,     207);
    CHECK_EQ("byron207_last.epoch_slot", info.epoch_slot,
             CARDANO_BYRON_EPOCH_SLOTS - 1);

    /* slot 4492799: still Byron (one slot before Shelley start) */
    info = cardano_info_for_slot(CARDANO_SHELLEY_START_SLOT - 1);
    CHECK_EQ("pre_shelley.epoch",      info.epoch,     207);
    CHECK_EQ("pre_shelley.abs_slot",   info.absolute_slot,
             CARDANO_SHELLEY_START_SLOT - 1);
}

static void test_byron_time_roundtrip(void)
{
    /* Byron genesis time -> slot 0 */
    cardano_info_t info = cardano_info_for_time((time_t)CARDANO_BYRON_GENESIS_TIME);
    CHECK_EQ("time_byron_genesis.epoch",     info.epoch,         0);
    CHECK_EQ("time_byron_genesis.abs_slot",  info.absolute_slot, 0);

    /* Pre-genesis time: must clamp to slot 0 */
    info = cardano_info_for_time((time_t)(CARDANO_BYRON_GENESIS_TIME - 1));
    CHECK_EQ("time_pre_genesis.abs_slot", info.absolute_slot, 0);

    /* Mid-epoch 0: 100 slots in = time + 100*20 s */
    time_t mid = (time_t)(CARDANO_BYRON_GENESIS_TIME + 100 * CARDANO_BYRON_SLOT_DURATION);
    info = cardano_info_for_time(mid);
    CHECK_EQ("time_byron_mid.epoch",      info.epoch,      0);
    CHECK_EQ("time_byron_mid.abs_slot",   info.absolute_slot, 100);
    CHECK_EQ("time_byron_mid.epoch_slot", info.epoch_slot, 100);
}

/* ------------------------------------------------------------------ */
/* Era boundary                                                         */
/* ------------------------------------------------------------------ */

static void test_era_boundary(void)
{
    /* slot 4492799 -> Byron epoch 207 */
    cardano_info_t before = cardano_info_for_slot(CARDANO_SHELLEY_START_SLOT - 1);
    CHECK_EQ("boundary_before.epoch", before.epoch, 207);

    /* slot 4492800 -> Shelley epoch 208 */
    cardano_info_t after = cardano_info_for_slot(CARDANO_SHELLEY_START_SLOT);
    CHECK_EQ("boundary_after.epoch",      after.epoch,         208);
    CHECK_EQ("boundary_after.epoch_slot", after.epoch_slot,    0);
    CHECK_EQ("boundary_after.abs_slot",   after.absolute_slot, CARDANO_SHELLEY_START_SLOT);
}

/* ------------------------------------------------------------------ */
/* Shelley era                                                          */
/* ------------------------------------------------------------------ */

static void test_shelley_start(void)
{
    cardano_info_t info = cardano_info_for_slot(CARDANO_SHELLEY_START_SLOT);
    CHECK_EQ("shelley_start.epoch",      info.epoch,         208);
    CHECK_EQ("shelley_start.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("shelley_start.abs_slot",   info.absolute_slot, CARDANO_SHELLEY_START_SLOT);
    CHECK_EQ("shelley_start.start_time", (uint64_t)info.epoch_start_time,
             CARDANO_SHELLEY_START_TIME);
    CHECK_EQ("shelley_start.end_time",   (uint64_t)info.epoch_end_time,
             CARDANO_SHELLEY_START_TIME
             + CARDANO_SHELLEY_EPOCH_SLOTS * CARDANO_SHELLEY_SLOT_DURATION);
}

static void test_shelley_epoch208_for_epoch(void)
{
    cardano_info_t info = cardano_info_for_epoch(208);
    CHECK_EQ("epoch208.epoch",      info.epoch,         208);
    CHECK_EQ("epoch208.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("epoch208.abs_slot",   info.absolute_slot, CARDANO_SHELLEY_START_SLOT);
}

static void test_shelley_epoch208_last_slot(void)
{
    uint64_t last = CARDANO_SHELLEY_START_SLOT + CARDANO_SHELLEY_EPOCH_SLOTS - 1;
    cardano_info_t info = cardano_info_for_slot(last);
    CHECK_EQ("epoch208_last.epoch",      info.epoch,     208);
    CHECK_EQ("epoch208_last.epoch_slot", info.epoch_slot,
             CARDANO_SHELLEY_EPOCH_SLOTS - 1);
    CHECK_EQ("epoch208_last.abs_slot",   info.absolute_slot, last);
}

static void test_shelley_epoch209(void)
{
    uint64_t expected_abs = CARDANO_SHELLEY_START_SLOT + CARDANO_SHELLEY_EPOCH_SLOTS;
    cardano_info_t info = cardano_info_for_epoch(209);
    CHECK_EQ("epoch209.epoch",      info.epoch,         209);
    CHECK_EQ("epoch209.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("epoch209.abs_slot",   info.absolute_slot, expected_abs);
    CHECK_EQ("epoch209.start_time", (uint64_t)info.epoch_start_time,
             CARDANO_SHELLEY_START_TIME
             + CARDANO_SHELLEY_EPOCH_SLOTS * CARDANO_SHELLEY_SLOT_DURATION);
}

static void test_shelley_mid_epoch(void)
{
    /* slot halfway through epoch 209 */
    uint64_t mid_slot = CARDANO_SHELLEY_START_SLOT
                        + CARDANO_SHELLEY_EPOCH_SLOTS       /* start of 209 */
                        + CARDANO_SHELLEY_EPOCH_SLOTS / 2;  /* midpoint     */
    cardano_info_t info = cardano_info_for_slot(mid_slot);
    CHECK_EQ("mid209.epoch",      info.epoch,     209);
    CHECK_EQ("mid209.epoch_slot", info.epoch_slot, CARDANO_SHELLEY_EPOCH_SLOTS / 2);
}

static void test_shelley_far_epoch(void)
{
    /* Epoch 500: well into Shelley */
    uint64_t shelley_n = 500 - CARDANO_SHELLEY_START_EPOCH; /* 292 */
    uint64_t expected_abs = CARDANO_SHELLEY_START_SLOT
                            + shelley_n * CARDANO_SHELLEY_EPOCH_SLOTS;
    cardano_info_t info = cardano_info_for_epoch(500);
    CHECK_EQ("epoch500.epoch",      info.epoch,         500);
    CHECK_EQ("epoch500.epoch_slot", info.epoch_slot,    0);
    CHECK_EQ("epoch500.abs_slot",   info.absolute_slot, expected_abs);
    CHECK_EQ("epoch500.start_time", (uint64_t)info.epoch_start_time,
             CARDANO_SHELLEY_START_TIME
             + shelley_n * CARDANO_SHELLEY_EPOCH_SLOTS * CARDANO_SHELLEY_SLOT_DURATION);
}

static void test_shelley_time_roundtrip(void)
{
    /* Shelley start time -> epoch 208, epoch_slot 0 */
    cardano_info_t info = cardano_info_for_time((time_t)CARDANO_SHELLEY_START_TIME);
    CHECK_EQ("time_shelley.epoch",      info.epoch,     208);
    CHECK_EQ("time_shelley.epoch_slot", info.epoch_slot,  0);
    CHECK_EQ("time_shelley.abs_slot",   info.absolute_slot, CARDANO_SHELLEY_START_SLOT);

    /* 1000 seconds into Shelley -> epoch_slot 1000 */
    info = cardano_info_for_time((time_t)(CARDANO_SHELLEY_START_TIME + 1000));
    CHECK_EQ("time_shelley+1000.epoch",      info.epoch,     208);
    CHECK_EQ("time_shelley+1000.epoch_slot", info.epoch_slot, 1000);
}

/* ------------------------------------------------------------------ */
/* Consistency: for_epoch vs for_slot                                   */
/* ------------------------------------------------------------------ */

static void test_epoch_slot_consistency(void)
{
    uint64_t epochs[] = {0, 1, 100, 207, 208, 209, 300, 500};
    size_t n = sizeof(epochs) / sizeof(epochs[0]);
    char label[64];

    for (size_t i = 0; i < n; i++) {
        cardano_info_t fe = cardano_info_for_epoch(epochs[i]);
        cardano_info_t fs = cardano_info_for_slot(fe.absolute_slot);

        snprintf(label, sizeof(label), "consistency[%" PRIu64 "].epoch", epochs[i]);
        CHECK_EQ(label, fs.epoch, fe.epoch);

        snprintf(label, sizeof(label), "consistency[%" PRIu64 "].epoch_slot", epochs[i]);
        CHECK_EQ(label, fs.epoch_slot, fe.epoch_slot);

        snprintf(label, sizeof(label), "consistency[%" PRIu64 "].start_time", epochs[i]);
        CHECK_EQ(label, (uint64_t)fs.epoch_start_time, (uint64_t)fe.epoch_start_time);
    }
}

/* ------------------------------------------------------------------ */
/* epoch_end_time sanity                                                */
/* ------------------------------------------------------------------ */

static void test_epoch_end_time(void)
{
    /* end_time must equal next epoch's start_time */
    cardano_info_t e208 = cardano_info_for_epoch(208);
    cardano_info_t e209 = cardano_info_for_epoch(209);
    CHECK_EQ("end208_eq_start209",
             (uint64_t)e208.epoch_end_time, (uint64_t)e209.epoch_start_time);

    cardano_info_t e0 = cardano_info_for_epoch(0);
    cardano_info_t e1 = cardano_info_for_epoch(1);
    CHECK_EQ("end0_eq_start1",
             (uint64_t)e0.epoch_end_time, (uint64_t)e1.epoch_start_time);

    /* end_time > start_time for every epoch */
    CHECK_EQ("end208_gt_start208",
             (uint64_t)e208.epoch_end_time > (uint64_t)e208.epoch_start_time, 1);
    CHECK_EQ("end0_gt_start0",
             (uint64_t)e0.epoch_end_time   > (uint64_t)e0.epoch_start_time,   1);
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(void)
{
    test_byron_genesis();
    test_byron_epoch0_last_slot();
    test_byron_epoch1();
    test_byron_last_epoch();
    test_byron_time_roundtrip();

    test_era_boundary();

    test_shelley_start();
    test_shelley_epoch208_for_epoch();
    test_shelley_epoch208_last_slot();
    test_shelley_epoch209();
    test_shelley_mid_epoch();
    test_shelley_far_epoch();
    test_shelley_time_roundtrip();

    test_epoch_slot_consistency();
    test_epoch_end_time();

    if (failures == 0)
        printf("All %d checks passed.\n", checks);
    else
        printf("%d/%d check(s) failed.\n", failures, checks);

    return failures > 0 ? 1 : 0;
}
