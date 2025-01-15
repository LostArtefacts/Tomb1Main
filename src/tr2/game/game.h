#pragma once

#include "global/types.h"

#include <libtrx/game/game.h>
#include <libtrx/game/gameflow/types.h>

bool Game_Start(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
void Game_End(void);
void Game_Suspend(void);
void Game_Resume(void);

GAME_FLOW_COMMAND Game_Control(bool demo_mode);
void Game_Draw(bool draw_overlay);

bool Game_IsPlayable(void);
void Game_ProcessInput(void);
