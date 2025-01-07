#pragma once

#include <libtrx/game/phase.h>

#include <stdint.h>

typedef struct {
    bool show_final_stats;
    double fade_in_time;
    double fade_out_time;
} PHASE_STATS_ARGS;

PHASE *Phase_Stats_Create(PHASE_STATS_ARGS args);
void Phase_Stats_Destroy(PHASE *phase);
