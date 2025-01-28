#pragma once

#include "./game_flow/types.h"

void Game_SetIsPlaying(bool is_playing);
bool Game_IsPlaying(void);

const GF_LEVEL *Game_GetCurrentLevel(void);
void Game_SetCurrentLevel(const GF_LEVEL *level);

bool Game_IsInGym(void);
bool Game_IsPlayable(void);

extern bool Game_Start(const GF_LEVEL *level, GF_SEQUENCE_CONTEXT seq_ctx);
extern void Game_End(void);
extern void Game_Suspend(void);
extern void Game_Resume(void);
extern GF_COMMAND Game_Control(bool demo_mode);
extern void Game_Draw(bool draw_overlay);
