#pragma once

#include "global/types.h"

#include <stdint.h>

bool GF_LoadFromFile(const char *file_name);
int32_t GF_LoadScriptFile(const char *fname);
int32_t GF_DoFrontendSequence(void);
int32_t GF_DoLevelSequence(int32_t level, GAMEFLOW_LEVEL_TYPE type);
int32_t GF_InterpretSequence(
    const int16_t *ptr, GAMEFLOW_LEVEL_TYPE type, int32_t seq_type);
void GF_ModifyInventory(int32_t level, int32_t type);

GAME_FLOW_DIR GF_StartDemo(int32_t level_num);
GAME_FLOW_DIR GF_StartGame(int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
GAME_FLOW_DIR GF_ShowInventory(INVENTORY_MODE mode);
GAME_FLOW_DIR GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);
