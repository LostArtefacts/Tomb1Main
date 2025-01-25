#pragma once

#include <libtrx/game/game.h>

void Game_ProcessInput(void);
GAME_FLOW_LEVEL *Game_GetCurrentLevel(void);
void Game_SetCurrentLevel(GAME_FLOW_LEVEL *level);
