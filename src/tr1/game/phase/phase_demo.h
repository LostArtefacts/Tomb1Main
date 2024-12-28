#pragma once

#include "game/phase/phase.h"

typedef struct {
    int32_t demo_num;
    bool resume_existing;
} PHASE_DEMO_ARGS;

extern PHASER g_DemoPhaser;
