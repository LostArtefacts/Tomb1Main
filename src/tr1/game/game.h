#pragma once

#include <libtrx/game/game.h>

bool Game_Start_Legacy(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND Game_Stop_Legacy(void);

void Game_ProcessInput(void);

GAME_FLOW_COMMAND Game_MainMenu(void);
