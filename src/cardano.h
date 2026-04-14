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
 *
 * Pre-Production testnet (--testnet-magic 1):
 *   Byron genesis: 2022-06-01 00:00:00 UTC  (Unix: 1654041600)
 *   Shelley start: epoch 4, slot 86400, 2022-06-21 00:00:00 UTC (Unix: 1655769600)
 *
 * Preview testnet (--testnet-magic 2):
 *   Byron genesis:  2022-10-25 00:00:00 UTC  (Unix: 1666656000)
 *   Byron era:      20 s/slot, 4320 slots/epoch (k=432), epoch 0
 *   Shelley start:  epoch 1, slot 4320, 2022-10-26 00:00:00 UTC (Unix: 1666742400)
 *   Shelley era:    1 s/slot, 86400 slots/epoch (1 day)
 */
#ifndef CARDANO_H
#define CARDANO_H

#include <stdint.h>
#include <time.h>

/* ------------------------------------------------------------------
 * Mainnet constants (kept for backward compatibility and test use)
 * ------------------------------------------------------------------ */
#define CARDANO_BYRON_GENESIS_TIME    UINT64_C(1506203091)  /* 2017-09-23 21:44:51 UTC */
#define CARDANO_BYRON_SLOT_DURATION   UINT64_C(20)          /* seconds per slot */
#define CARDANO_BYRON_EPOCH_SLOTS     UINT64_C(21600)       /* slots per epoch  */

#define CARDANO_SHELLEY_START_EPOCH   UINT64_C(208)
#define CARDANO_SHELLEY_START_SLOT    UINT64_C(4492800)
#define CARDANO_SHELLEY_START_TIME    UINT64_C(1596059091)  /* 2020-07-29 21:44:51 UTC */
#define CARDANO_SHELLEY_SLOT_DURATION UINT64_C(1)           /* seconds per slot */
#define CARDANO_SHELLEY_EPOCH_SLOTS   UINT64_C(432000)      /* slots per epoch  */

/* ------------------------------------------------------------------
 * Network parameter set
 * ------------------------------------------------------------------ */
typedef struct {
    const char *name;
    uint64_t    byron_genesis_time;   /* Unix timestamp of Byron genesis    */
    uint64_t    byron_slot_duration;  /* seconds per Byron slot             */
    uint64_t    byron_epoch_slots;    /* slots per Byron epoch              */
    uint64_t    shelley_start_epoch;  /* first Shelley epoch number         */
    uint64_t    shelley_start_slot;   /* absolute slot of Shelley hard fork */
    uint64_t    shelley_start_time;   /* Unix timestamp of Shelley HF       */
    uint64_t    shelley_slot_duration;/* seconds per Shelley slot           */
    uint64_t    shelley_epoch_slots;  /* slots per Shelley epoch            */
} cardano_network_params_t;

/* Predefined network configurations */
extern const cardano_network_params_t CARDANO_MAINNET;
extern const cardano_network_params_t CARDANO_PREPROD;
extern const cardano_network_params_t CARDANO_PREVIEW;

/*
 * Select the active network for all subsequent calculations.
 * Defaults to CARDANO_MAINNET if never called.
 */
void cardano_set_network(const cardano_network_params_t *net);

/* Return the name of the currently active network (e.g. "mainnet"). */
const char *cardano_network_name(void);

/* ------------------------------------------------------------------
 * Result type
 * ------------------------------------------------------------------ */
typedef struct {
    uint64_t epoch;
    uint64_t absolute_slot;
    uint64_t epoch_slot;
    uint64_t epoch_slots_total;      /* total slots in this epoch          */
    time_t   epoch_start_time;       /* UTC Unix timestamp of epoch start  */
    time_t   epoch_end_time;         /* UTC Unix timestamp of epoch end    */
} cardano_info_t;

/* ------------------------------------------------------------------
 * Query functions (use the active network set via cardano_set_network)
 * ------------------------------------------------------------------ */

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
