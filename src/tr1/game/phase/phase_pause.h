#pragma once

#include "game/phase/phase.h"

typedef struct {
    PHASE_ENUM phase_to_return_to;
    void *phase_arg;
} PHASE_PAUSE_ARGS;

extern PHASER g_PausePhaser;
