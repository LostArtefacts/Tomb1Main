#pragma once

#include "global/types.h"

#include <libtrx/game/game.h>

bool Game_Start(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND Game_Stop(void);

void Game_ProcessInput(void);

void Game_Draw(bool draw_overlay);

GAME_FLOW_COMMAND Game_MainMenu(void);

bool Game_IsPlayable(void);
