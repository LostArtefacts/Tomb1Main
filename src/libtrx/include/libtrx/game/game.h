#pragma once

#include "./game_flow/types.h"

void Game_SetIsPlaying(bool is_playing);
bool Game_IsPlaying(void);

GAME_FLOW_LEVEL *Game_GetCurrentLevel(void);
void Game_SetCurrentLevel(GAME_FLOW_LEVEL *level);

extern bool Game_Start(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
extern void Game_End(void);
extern void Game_Suspend(void);
extern void Game_Resume(void);
extern GAME_FLOW_COMMAND Game_Control(bool demo_mode);
extern void Game_Draw(bool draw_overlay);

extern bool Game_IsInGym(void);
extern bool Game_IsPlayable(void);
extern GAME_FLOW_LEVEL_TYPE Game_GetCurrentLevelType(void);
extern int32_t Game_GetCurrentLevelNum(void);
