/*
 * cardano.h - Cardano blockchain epoch/slot calculations
 *
 * Mainnet parameters:
 *   Byron era (epochs 0-207):
 *     Genesis:      2017-09-23 21:44:51 UTC  (Unix: 1506203091)
 *     Slot length:  20 seconds
 *     Epoch length: 21600 slots  (5 days)
 *
 *   Shelley+ era (epochs 208+):
 *     Hard fork:    2020-07-29 21:44:51 UTC  (Unix: 1596059091)
 *     Start slot:   4492800 (absolute)
 *     Slot length:  1 second
 *     Epoch length: 432000 slots  (5 days)
 */
#ifndef CARDANO_H
#define CARDANO_H

#include <stdint.h>
#include <time.h>

#define CARDANO_BYRON_GENESIS_TIME    UINT64_C(1506203091)  /* 2017-09-23 21:44:51 UTC */
#define CARDANO_BYRON_SLOT_DURATION   UINT64_C(20)          /* seconds per slot */
#define CARDANO_BYRON_EPOCH_SLOTS     UINT64_C(21600)       /* slots per epoch  */

#define CARDANO_SHELLEY_START_EPOCH   UINT64_C(208)
#define CARDANO_SHELLEY_START_SLOT    UINT64_C(4492800)
#define CARDANO_SHELLEY_START_TIME    UINT64_C(1596059091)  /* 2020-07-29 21:44:51 UTC */
#define CARDANO_SHELLEY_SLOT_DURATION UINT64_C(1)           /* seconds per slot */
#define CARDANO_SHELLEY_EPOCH_SLOTS   UINT64_C(432000)      /* slots per epoch  */

typedef struct {
    uint64_t epoch;
    uint64_t absolute_slot;
    uint64_t epoch_slot;
    time_t   epoch_start_time; /* UTC Unix timestamp of epoch start */
    time_t   epoch_end_time;   /* UTC Unix timestamp of epoch end   */
} cardano_info_t;

/* Info based on the current wall-clock time */
cardano_info_t cardano_current_info(void);

/* Info for the start of a given epoch */
cardano_info_t cardano_info_for_epoch(uint64_t epoch);

/* Info for a given absolute slot */
cardano_info_t cardano_info_for_slot(uint64_t slot);

/* Info for a given epoch-slot within the current epoch */
cardano_info_t cardano_info_for_current_epoch_slot(uint64_t epoch_slot);

/* Info for a given UTC Unix timestamp */
cardano_info_t cardano_info_for_time(time_t t);

#endif /* CARDANO_H */
