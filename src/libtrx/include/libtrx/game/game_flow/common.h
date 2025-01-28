#pragma once

#include "./types.h"

bool GF_IsGymEnabled(void);

void GF_OverrideCommand(GAME_FLOW_COMMAND action);
GAME_FLOW_COMMAND GF_GetOverrideCommand(void);

const GAME_FLOW_LEVEL *GF_GetCurrentLevel(void);
void GF_SetCurrentLevel(const GAME_FLOW_LEVEL *level);

void GF_Shutdown(void);

extern int32_t GF_GetLevelCount(GAME_FLOW_LEVEL_TYPE level_type);
extern int32_t GF_GetDemoCount(void);
extern void GF_SetLevelTitle(GAME_FLOW_LEVEL *level, const char *title);
extern int32_t GF_GetGymLevelNum(void);
extern GAME_FLOW_LEVEL *GF_GetLevel(
    int32_t num, GAME_FLOW_LEVEL_TYPE level_type);
