#pragma once

#include "./types.h"

bool GF_IsGymEnabled(void);

void GF_OverrideCommand(GAME_FLOW_COMMAND action);
GAME_FLOW_COMMAND GF_GetOverrideCommand(void);

extern int32_t GF_GetLevelCount(void);
extern int32_t GF_GetDemoCount(void);
extern const char *GF_GetLevelTitle(int32_t level_num);
extern void GF_SetLevelTitle(int32_t level_num, const char *title);
extern int32_t GF_GetGymLevelNum(void);
