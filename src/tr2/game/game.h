#pragma once

#include "global/types.h"

#include <libtrx/game/game.h>

GAME_FLOW_COMMAND Game_Control(int32_t num_frames, bool demo_mode);
void Game_Draw(bool draw_overlay);
bool Game_IsPlayable(void);
void Game_ProcessInput(void);
