#pragma once

#include "global/types.h"

// TODO: consolidate with STATISTICS_INFO
typedef struct {
    uint32_t timer;
    uint32_t ammo_used;
    uint32_t ammo_hits;
    uint32_t distance;
    uint32_t kills;
    uint8_t found_secrets; // this is no longer a bitmask
    uint8_t total_secrets; // this is not present in STATISTICS_INFO
    uint8_t medipacks;
} FINAL_STATS;

void Stats_StartTimer(void);
void Stats_UpdateTimer(void);
FINAL_STATS Stats_ComputeFinalStats(void);
