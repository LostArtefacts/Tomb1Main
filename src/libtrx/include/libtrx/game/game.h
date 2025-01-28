#pragma once

#include "./game_flow/types.h"

void Game_SetIsPlaying(bool is_playing);
bool Game_IsPlaying(void);

const GAME_FLOW_LEVEL *Game_GetCurrentLevel(void);
void Game_SetCurrentLevel(const GAME_FLOW_LEVEL *level);

bool Game_IsInGym(void);
bool Game_IsPlayable(void);

extern bool Game_Start(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);
extern void Game_End(void);
extern void Game_Suspend(void);
extern void Game_Resume(void);
extern GAME_FLOW_COMMAND Game_Control(bool demo_mode);
extern void Game_Draw(bool draw_overlay);
