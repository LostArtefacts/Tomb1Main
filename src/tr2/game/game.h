#pragma once

#include "global/types.h"

int32_t Game_Control(int32_t nframes, bool demo_mode);
int32_t Game_ControlCinematic(void);
int32_t Game_Draw(void);
int32_t Game_DrawCinematic(void);
int16_t Game_Start(int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
GAME_FLOW_DIR Game_Loop(bool demo_mode);
int32_t Game_LoopCinematic(int32_t level_num);
bool Game_IsPlayable(void);
void Game_ProcessInput(void);
