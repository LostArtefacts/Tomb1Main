#pragma once

#include "./types.h"

#include <libtrx/game/game_flow/sequencer.h>

GAME_FLOW_COMMAND GF_StartGame(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);

GAME_FLOW_COMMAND GF_InterpretSequence(
    const GAME_FLOW_SEQUENCE *sequence, GAME_FLOW_LEVEL_TYPE type);

bool GF_DoFrontendSequence(void);
GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num);
GAME_FLOW_COMMAND GF_DoLevelSequence(
    int32_t start_level, GAME_FLOW_LEVEL_TYPE type);
