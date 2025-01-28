#pragma once

#include "./types.h"

void GF_Shutdown(void);

void GF_OverrideCommand(GAME_FLOW_COMMAND action);
GAME_FLOW_COMMAND GF_GetOverrideCommand(void);

GAME_FLOW_LEVEL_TABLE_TYPE GF_GetLevelTableType(
    const GAME_FLOW_LEVEL_TYPE level_type);
const GAME_FLOW_LEVEL_TABLE *GF_GetLevelTable(
    GAME_FLOW_LEVEL_TABLE_TYPE level_type);

const GAME_FLOW_LEVEL *GF_GetCurrentLevel(void);
const GAME_FLOW_LEVEL *GF_GetTitleLevel(void);
const GAME_FLOW_LEVEL *GF_GetGymLevel(void);
const GAME_FLOW_LEVEL *GF_GetFirstLevel(void);
const GAME_FLOW_LEVEL *GF_GetLastLevel(void);
const GAME_FLOW_LEVEL *GF_GetLevel(
    GAME_FLOW_LEVEL_TABLE_TYPE level_table_type, int32_t num);
const GAME_FLOW_LEVEL *GF_GetLevelAfter(const GAME_FLOW_LEVEL *level);
const GAME_FLOW_LEVEL *GF_GetLevelBefore(const GAME_FLOW_LEVEL *level);

void GF_SetCurrentLevel(const GAME_FLOW_LEVEL *level);
void GF_SetLevelTitle(GAME_FLOW_LEVEL *level, const char *title);
