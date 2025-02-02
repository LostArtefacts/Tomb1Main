#pragma once

#include "./types.h"

extern void GF_Init(void);
void GF_Shutdown(void);

void GF_OverrideCommand(GF_COMMAND action);
GF_COMMAND GF_GetOverrideCommand(void);

GF_LEVEL_TABLE_TYPE GF_GetLevelTableType(const GF_LEVEL_TYPE level_type);
const GF_LEVEL_TABLE *GF_GetLevelTable(GF_LEVEL_TABLE_TYPE level_type);

const GF_LEVEL *GF_GetCurrentLevel(void);
const GF_LEVEL *GF_GetTitleLevel(void);
const GF_LEVEL *GF_GetGymLevel(void);
const GF_LEVEL *GF_GetFirstLevel(void);
const GF_LEVEL *GF_GetLastLevel(void);
const GF_LEVEL *GF_GetLevel(GF_LEVEL_TABLE_TYPE level_table_type, int32_t num);
const GF_LEVEL *GF_GetLevelAfter(const GF_LEVEL *level);
const GF_LEVEL *GF_GetLevelBefore(const GF_LEVEL *level);

void GF_SetCurrentLevel(const GF_LEVEL *level);
void GF_SetLevelTitle(GF_LEVEL *level, const char *title);
