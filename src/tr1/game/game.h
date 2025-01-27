#pragma once

#include <libtrx/game/game.h>

bool Game_Start_Legacy(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);
GAME_FLOW_COMMAND Game_Stop_Legacy(void);

void Game_ProcessInput(void);

GAME_FLOW_COMMAND Game_MainMenu(void);
