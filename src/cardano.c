/*
 * cardano.c - Cardano blockchain epoch/slot calculations
 *
 * Copyright (c) 2026 Neal Allen Morrison <nmorrison@magister-technicus.de>
 * Magister Technicus GmbH
 * SPDX-License-Identifier: MIT
 */
#include "cardano.h"
#include <time.h>

/*
 * Core conversion: absolute slot -> cardano_info_t.
 * All public functions ultimately call this.
 */
static cardano_info_t slot_to_info(uint64_t absolute_slot)
{
    cardano_info_t info;
    info.absolute_slot = absolute_slot;

    if (absolute_slot < CARDANO_SHELLEY_START_SLOT) {
        /* Byron era */
        uint64_t byron_epoch     = absolute_slot / CARDANO_BYRON_EPOCH_SLOTS;
        info.epoch               = byron_epoch;
        info.epoch_slot          = absolute_slot % CARDANO_BYRON_EPOCH_SLOTS;
        info.epoch_start_time    = (time_t)(CARDANO_BYRON_GENESIS_TIME
            + byron_epoch * CARDANO_BYRON_EPOCH_SLOTS * CARDANO_BYRON_SLOT_DURATION);
        info.epoch_end_time      = (time_t)((uint64_t)info.epoch_start_time
            + CARDANO_BYRON_EPOCH_SLOTS * CARDANO_BYRON_SLOT_DURATION);
    } else {
        /* Shelley+ era */
        uint64_t shelley_slots   = absolute_slot - CARDANO_SHELLEY_START_SLOT;
        uint64_t shelley_epoch   = shelley_slots / CARDANO_SHELLEY_EPOCH_SLOTS;
        info.epoch               = CARDANO_SHELLEY_START_EPOCH + shelley_epoch;
        info.epoch_slot          = shelley_slots % CARDANO_SHELLEY_EPOCH_SLOTS;
        info.epoch_start_time    = (time_t)(CARDANO_SHELLEY_START_TIME
            + shelley_epoch * CARDANO_SHELLEY_EPOCH_SLOTS
                             * CARDANO_SHELLEY_SLOT_DURATION);
        info.epoch_end_time      = (time_t)((uint64_t)info.epoch_start_time
            + CARDANO_SHELLEY_EPOCH_SLOTS * CARDANO_SHELLEY_SLOT_DURATION);
    }

    return info;
}

/*
 * Convert a UTC Unix timestamp to an absolute Cardano slot.
 */
static uint64_t time_to_slot(time_t t)
{
    uint64_t ut = (uint64_t)t;

    if (ut < CARDANO_SHELLEY_START_TIME) {
        /* Byron era: one slot every 20 seconds */
        if (ut < CARDANO_BYRON_GENESIS_TIME)
            ut = CARDANO_BYRON_GENESIS_TIME; /* clamp to genesis */
        return (ut - CARDANO_BYRON_GENESIS_TIME) / CARDANO_BYRON_SLOT_DURATION;
    }

    /* Shelley+ era: one slot per second */
    return CARDANO_SHELLEY_START_SLOT + (ut - CARDANO_SHELLEY_START_TIME);
}

cardano_info_t cardano_current_info(void)
{
    return slot_to_info(time_to_slot(time(NULL)));
}

cardano_info_t cardano_info_for_epoch(uint64_t epoch)
{
    uint64_t absolute_slot;

    if (epoch < CARDANO_SHELLEY_START_EPOCH) {
        absolute_slot = epoch * CARDANO_BYRON_EPOCH_SLOTS;
    } else {
        uint64_t shelley_epoch = epoch - CARDANO_SHELLEY_START_EPOCH;
        absolute_slot = CARDANO_SHELLEY_START_SLOT
                      + shelley_epoch * CARDANO_SHELLEY_EPOCH_SLOTS;
    }

    return slot_to_info(absolute_slot);
}

cardano_info_t cardano_info_for_slot(uint64_t slot)
{
    return slot_to_info(slot);
}

cardano_info_t cardano_info_for_current_epoch_slot(uint64_t epoch_slot)
{
    cardano_info_t cur       = cardano_current_info();
    uint64_t epoch_start_abs = cur.absolute_slot - cur.epoch_slot;
    return slot_to_info(epoch_start_abs + epoch_slot);
}

cardano_info_t cardano_info_for_time(time_t t)
{
    return slot_to_info(time_to_slot(t));
}
