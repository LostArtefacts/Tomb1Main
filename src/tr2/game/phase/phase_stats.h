#pragma once

#include "game/phase/common.h"

#include <stdint.h>

typedef struct {
    bool show_final_stats;
    int32_t fade_in_time;
    int32_t fade_out_time;
} PHASE_STATS_ARGS;

PHASE *Phase_Stats_Create(PHASE_STATS_ARGS args);
void Phase_Stats_Destroy(PHASE *phase);
