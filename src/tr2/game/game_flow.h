#pragma once

#include "game/game_flow/common.h"
#include "game/game_flow/inventory.h"
#include "game/game_flow/sequencer.h"
#include "game/game_flow/vars.h"

bool GF_LoadFromFile(const char *file_name);
bool GF_LoadScriptFile(const char *fname);
bool GF_DoFrontendSequence(void);
GAME_FLOW_COMMAND GF_DoLevelSequence(int32_t level, GAME_FLOW_LEVEL_TYPE type);
GAME_FLOW_COMMAND GF_InterpretSequence(
    const int16_t *ptr, GAME_FLOW_LEVEL_TYPE type);

GAME_FLOW_COMMAND GF_TranslateScriptCommand(uint16_t dir);
