#pragma once

#include "global/types.h"

#include <libtrx/game/phase/control.h>

#include <stdint.h>

typedef enum {
    PHASE_NULL,
    PHASE_GAME,
} PHASE_ENUM;

typedef void (*PHASER_START)(const void *args);
typedef void (*PHASER_END)(void);
typedef PHASE_CONTROL (*PHASER_CONTROL)(int32_t nframes);
typedef void (*PHASER_DRAW)(void);
typedef int32_t (*PHASER_WAIT)(void);

typedef struct {
    PHASER_START start;
    PHASER_END end;
    PHASER_CONTROL control;
    PHASER_DRAW draw;
    PHASER_WAIT wait;
} PHASER;

PHASE_ENUM Phase_Get(void);

// Sets the next phase to run.
// args are passed to the subsequent PHASER->start callback.
// Note that they must be allocated on the heap and will be
// immediately freed by the phaser module upon completing the start
// routine.
void Phase_Set(PHASE_ENUM phase, const void *args);

GAME_FLOW_COMMAND Phase_Run(void);
