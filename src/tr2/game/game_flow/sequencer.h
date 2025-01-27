#pragma once

#include "./types.h"

#include <libtrx/game/game_flow/sequencer.h>

GAME_FLOW_COMMAND GF_InterpretSequence(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);

bool GF_DoFrontendSequence(void);
GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num);
GAME_FLOW_COMMAND GF_DoCutsceneSequence(int32_t cutscene_num);
GAME_FLOW_COMMAND GF_DoLevelSequence(
    const GAME_FLOW_LEVEL *start_level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);
