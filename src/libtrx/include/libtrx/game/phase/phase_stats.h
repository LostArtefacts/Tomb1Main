#pragma once

#include "../output.h"
#include "./types.h"

typedef struct {
    BACKGROUND_TYPE background_type;
    const char *background_path;
    bool show_final_stats;
    int32_t level_num;
} PHASE_STATS_ARGS;

PHASE *Phase_Stats_Create(PHASE_STATS_ARGS args);
void Phase_Stats_Destroy(PHASE *phase);
