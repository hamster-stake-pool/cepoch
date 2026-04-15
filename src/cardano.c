/*
 * cardano.c - Cardano blockchain epoch/slot calculations
 *
 * Copyright (c) 2026 Neal Allen Morrison <nmorrison@magister-technicus.de>
 * Magister Technicus GmbH
 * SPDX-License-Identifier: MIT
 */
#include "cardano.h"
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------
 * Predefined network configurations
 * ------------------------------------------------------------------ */

const cardano_network_params_t CARDANO_MAINNET = {
    "mainnet",
    UINT64_C(1506203091),   /* byron_genesis_time:    2017-09-23 21:44:51 UTC */
    UINT64_C(20),           /* byron_slot_duration:   20 s/slot               */
    UINT64_C(21600),        /* byron_epoch_slots:     21600 slots/epoch        */
    UINT64_C(208),          /* shelley_start_epoch:   epoch 208                */
    UINT64_C(4492800),      /* shelley_start_slot:    slot 4492800             */
    UINT64_C(1596059091),   /* shelley_start_time:    2020-07-29 21:44:51 UTC  */
    UINT64_C(1),            /* shelley_slot_duration: 1 s/slot                 */
    UINT64_C(432000)        /* shelley_epoch_slots:   432000 slots/epoch       */
};

const cardano_network_params_t CARDANO_PREPROD = {
    "preprod",
    UINT64_C(1648771200),   /* byron_genesis_time:    2022-04-01 00:00:00 UTC  */
    UINT64_C(20),           /* byron_slot_duration:   20 s/slot                */
    UINT64_C(21600),        /* byron_epoch_slots:     21600 slots/epoch        */
    UINT64_C(4),            /* shelley_start_epoch:   epoch 4                  */
    UINT64_C(86400),        /* shelley_start_slot:    slot 86400 (4*21600)     */
    UINT64_C(1650499200),   /* shelley_start_time:    2022-04-21 00:00:00 UTC  */
    UINT64_C(1),            /* shelley_slot_duration: 1 s/slot                 */
    UINT64_C(432000)        /* shelley_epoch_slots:   432000 slots/epoch       */
};

/*
 * Preview testnet (network magic 2):
 *   Pure Shelley from slot 0; no Byron era.
 *   Shelley genesis: 2022-06-20 00:00:00 UTC  (Unix: 1655683200)
 *   Epoch counter starts at 4 (epoch 0-3 do not exist on-chain).
 *   slot length:  1 s/slot
 *   epoch length: 432000 slots (5 days)
 *
 * Source: shelley-genesis.json (systemStart, slotLength, epochLength).
 */
const cardano_network_params_t CARDANO_PREVIEW = {
    "preview",
    UINT64_C(1655683200),   /* byron_genesis_time:    same as shelley (no Byron era) */
    UINT64_C(20),           /* byron_slot_duration:   unused                    */
    UINT64_C(432000),       /* byron_epoch_slots:     unused                    */
    UINT64_C(4),            /* shelley_start_epoch:   epoch counter starts at 4 */
    UINT64_C(0),            /* shelley_start_slot:    slot 0 (no Byron era)     */
    UINT64_C(1655683200),   /* shelley_start_time:    2022-06-20 00:00:00 UTC   */
    UINT64_C(1),            /* shelley_slot_duration: 1 s/slot                  */
    UINT64_C(432000)        /* shelley_epoch_slots:   432000 slots/epoch        */
};

/* ------------------------------------------------------------------
 * Active network (default: mainnet)
 * ------------------------------------------------------------------ */
static cardano_network_params_t g_network = {
    "mainnet",
    UINT64_C(1506203091),
    UINT64_C(20),
    UINT64_C(21600),
    UINT64_C(208),
    UINT64_C(4492800),
    UINT64_C(1596059091),
    UINT64_C(1),
    UINT64_C(432000)
};

void cardano_set_network(const cardano_network_params_t *net)
{
    g_network = *net;
}

const char *cardano_network_name(void)
{
    return g_network.name;
}

/* ------------------------------------------------------------------
 * Core conversion: absolute slot -> cardano_info_t.
 * All public query functions ultimately call this.
 * ------------------------------------------------------------------ */
static cardano_info_t slot_to_info(uint64_t absolute_slot)
{
    const cardano_network_params_t *net = &g_network;
    cardano_info_t info;
    info.absolute_slot = absolute_slot;

    if (absolute_slot < net->shelley_start_slot) {
        /* Byron era */
        uint64_t byron_epoch   = absolute_slot / net->byron_epoch_slots;
        info.epoch             = byron_epoch;
        info.epoch_slot        = absolute_slot % net->byron_epoch_slots;
        info.epoch_slots_total = net->byron_epoch_slots;
        info.epoch_start_time  = (time_t)(net->byron_genesis_time
            + byron_epoch * net->byron_epoch_slots * net->byron_slot_duration);
        info.epoch_end_time    = (time_t)((uint64_t)info.epoch_start_time
            + net->byron_epoch_slots * net->byron_slot_duration);
    } else {
        /* Shelley+ era */
        uint64_t shelley_slots = absolute_slot - net->shelley_start_slot;
        uint64_t shelley_epoch = shelley_slots / net->shelley_epoch_slots;
        info.epoch             = net->shelley_start_epoch + shelley_epoch;
        info.epoch_slot        = shelley_slots % net->shelley_epoch_slots;
        info.epoch_slots_total = net->shelley_epoch_slots;
        info.epoch_start_time  = (time_t)(net->shelley_start_time
            + shelley_epoch * net->shelley_epoch_slots
                             * net->shelley_slot_duration);
        info.epoch_end_time    = (time_t)((uint64_t)info.epoch_start_time
            + net->shelley_epoch_slots * net->shelley_slot_duration);
    }

    return info;
}

/*
 * Convert a UTC Unix timestamp to an absolute Cardano slot.
 */
static uint64_t time_to_slot(time_t t)
{
    const cardano_network_params_t *net = &g_network;
    uint64_t ut = (uint64_t)t;

    if (ut < net->shelley_start_time) {
        /* Byron era: one slot every byron_slot_duration seconds */
        if (ut < net->byron_genesis_time)
            ut = net->byron_genesis_time; /* clamp to genesis */
        return (ut - net->byron_genesis_time) / net->byron_slot_duration;
    }

    /* Shelley+ era */
    return net->shelley_start_slot
         + (ut - net->shelley_start_time) / net->shelley_slot_duration;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

cardano_info_t cardano_current_info(void)
{
    return slot_to_info(time_to_slot(time(NULL)));
}

cardano_info_t cardano_info_for_epoch(uint64_t epoch)
{
    const cardano_network_params_t *net = &g_network;
    uint64_t absolute_slot;

    if (epoch < net->shelley_start_epoch) {
        absolute_slot = epoch * net->byron_epoch_slots;
    } else {
        uint64_t shelley_epoch = epoch - net->shelley_start_epoch;
        absolute_slot = net->shelley_start_slot
                      + shelley_epoch * net->shelley_epoch_slots;
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
