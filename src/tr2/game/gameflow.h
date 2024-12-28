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
