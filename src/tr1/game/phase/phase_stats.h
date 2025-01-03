#pragma once

#include "game/phase/phase.h"
#include "global/types.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t level_num;
    const char *background_path;
    bool total;
    GAME_FLOW_LEVEL_TYPE level_type;
} PHASE_STATS_ARGS;

extern PHASER g_StatsPhaser;
