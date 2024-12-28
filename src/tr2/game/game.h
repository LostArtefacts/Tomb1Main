#pragma once

#include "global/types.h"

GAME_FLOW_DIR Game_Control(int32_t num_frames, bool demo_mode);
void Game_Draw(void);
bool Game_IsPlayable(void);
void Game_ProcessInput(void);
