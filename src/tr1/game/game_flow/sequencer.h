#pragma once

#include "./types.h"

#include <libtrx/game/game_flow/sequencer.h>

GAME_FLOW_COMMAND
GF_InterpretSequence(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND
GF_StorySoFar(int32_t level_num, int32_t savegame_level);
GAME_FLOW_COMMAND GF_PlayAvailableStory(int32_t slot_num);

GAME_FLOW_COMMAND GF_LoadLevel(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_PlayLevel(
    int32_t demo_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_PlayDemo(int32_t demo_num);
GAME_FLOW_COMMAND GF_PlayCutscene(int32_t level_num);
