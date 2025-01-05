#pragma once

#include <libtrx/game/phase/types.h>

typedef struct {
    const char *background_path;
    bool show_final_stats;
    int32_t level_num;
    GAME_FLOW_LEVEL_TYPE level_type;
} PHASE_STATS_ARGS;

PHASE *Phase_Stats_Create(PHASE_STATS_ARGS args);
void Phase_Stats_Destroy(PHASE *phase);
