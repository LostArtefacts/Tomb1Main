#pragma once

#include "global/types.h"

#include <libtrx/game/inventory_ring/types.h>

#include <stdint.h>

bool GF_LoadFromFile(const char *file_name);
bool GF_LoadScriptFile(const char *fname);
bool GF_DoFrontendSequence(void);
GAME_FLOW_COMMAND GF_DoLevelSequence(int32_t level, GAME_FLOW_LEVEL_TYPE type);
GAME_FLOW_COMMAND GF_InterpretSequence(
    const int16_t *ptr, GAME_FLOW_LEVEL_TYPE type);
void GF_ModifyInventory(int32_t level, int32_t type);

GAME_FLOW_COMMAND GF_StartDemo(int32_t level_num);
GAME_FLOW_COMMAND GF_StartGame(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_ShowInventory(INVENTORY_MODE mode);
GAME_FLOW_COMMAND GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);

GAME_FLOW_COMMAND GF_TranslateScriptCommand(uint16_t dir);
