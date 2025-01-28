#pragma once

#include <libtrx/game/game.h>

bool Game_Start_Legacy(const GF_LEVEL *level, GF_SEQUENCE_CONTEXT seq_ctx);
GF_COMMAND Game_Stop_Legacy(void);

void Game_ProcessInput(void);

GF_COMMAND Game_MainMenu(void);
