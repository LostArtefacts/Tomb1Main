#pragma once

#include "./gameflow/types.h"

#include <stdbool.h>

void Game_SetIsPlaying(bool is_playing);
bool Game_IsPlaying(void);

extern void Game_Draw(bool draw_overlay);
extern bool Game_IsPlayable(void);
extern bool Game_IsExiting(void);
extern GAME_FLOW_LEVEL_TYPE Game_GetCurrentLevelType(void);
extern int32_t Game_GetCurrentLevelNum(void);
