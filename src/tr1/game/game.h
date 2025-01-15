#pragma once

#include "global/types.h"

#include <libtrx/game/game.h>

bool Game_Start_Legacy(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND Game_Stop_Legacy(void);

bool Game_Start(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
void Game_End(void);

void Game_Suspend(void);
void Game_Resume(void);

GAME_FLOW_COMMAND Game_Control(bool demo_mode);

void Game_Draw(bool draw_overlay);

void Game_ProcessInput(void);

GAME_FLOW_COMMAND Game_MainMenu(void);

bool Game_IsPlayable(void);
