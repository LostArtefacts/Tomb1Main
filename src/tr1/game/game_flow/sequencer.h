#pragma once

#include "./types.h"

#include <libtrx/game/game_flow/sequencer.h>

GAME_FLOW_COMMAND
GF_InterpretSequence(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);

GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num);
GAME_FLOW_COMMAND GF_DoCutsceneSequence(int32_t cutscene_num);
GAME_FLOW_COMMAND GF_PlayAvailableStory(int32_t slot_num);

GAME_FLOW_COMMAND GF_LoadLevel(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_PlayLevel(
    int32_t demo_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_PlayDemo(int32_t demo_num);
