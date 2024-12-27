#pragma once

#include "global/types.h"

typedef enum {
    PHASE_ACTION_CONTINUE,
    PHASE_ACTION_NO_WAIT,
    PHASE_ACTION_END,
} PHASE_ACTION;

typedef struct {
    PHASE_ACTION action;
    GAME_FLOW_DIR dir;
} PHASE_CONTROL;

typedef PHASE_CONTROL (*PHASE_START_FUNC)(PHASE *phase);
typedef void (*PHASE_END_FUNC)(PHASE *phase);
typedef PHASE_CONTROL (*PHASE_CONTROL_FUNC)(PHASE *phase, int32_t num_frames);
typedef void (*PHASE_DRAW_FUNC)(PHASE *phase);
typedef int32_t (*PHASE_WAIT_FUNC)(PHASE *phase);

typedef struct PHASE {
    PHASE_START_FUNC start;
    PHASE_END_FUNC end;
    PHASE_CONTROL_FUNC control;
    PHASE_DRAW_FUNC draw;
    PHASE_WAIT_FUNC wait;
    void *priv;
} PHASE;
