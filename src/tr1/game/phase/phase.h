#pragma once

#include "global/types.h"

#include <stdint.h>

// Status returned upon every logical frame by the control routine.
// - Set .end = false to keep the phase loop spinning.
// - Set .end = true to end the current phase.
//
// To continue executing current game sequence, .command.action member should
// be set to GF_NOOP. To break out of the current sequence and switch to a
// different game flow action, .command.action should be set to the action to
// run.
//
// It does not make sense to return both .end = false and .command.
typedef struct {
    bool end;
    GAME_FLOW_COMMAND command;
} PHASE_CONTROL;

typedef enum {
    PHASE_NULL,
    PHASE_GAME,
    PHASE_DEMO,
    PHASE_CUTSCENE,
    PHASE_PAUSE,
    PHASE_PICTURE,
    PHASE_STATS,
    PHASE_INVENTORY,
    PHASE_PHOTO_MODE,
} PHASE;

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

PHASE Phase_Get(void);

// Sets the next phase to run.
// args are passed to the subsequent PHASER->start callback.
// Note that they must be allocated on the heap and will be
// immediately freed by the phaser module upon completing the start
// routine.
void Phase_Set(PHASE phase, const void *args);

GAME_FLOW_COMMAND Phase_Run(void);
