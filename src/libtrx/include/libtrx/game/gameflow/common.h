#pragma once

#include "./types.h"

extern int32_t GameFlow_GetLevelCount(void);
extern int32_t GameFlow_GetDemoCount(void);
extern const char *GameFlow_GetLevelFileName(int32_t level_num);
extern const char *GameFlow_GetLevelTitle(int32_t level_num);
extern int32_t GameFlow_GetGymLevelNumber(void);

extern void GameFlow_OverrideCommand(GAME_FLOW_COMMAND action);
extern GAME_FLOW_COMMAND GameFlow_GetOverrideCommand(void);
