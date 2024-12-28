#pragma once

#include "global/types.h"

GAME_FLOW_DIR Game_Control(int32_t num_frames, bool demo_mode);
GAME_FLOW_DIR Game_ControlRaw(int32_t num_frames, bool demo_mode);
int32_t Game_Draw(void);
int16_t Game_Start(int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
GAME_FLOW_DIR Game_Loop(bool demo_mode);
bool Game_IsPlayable(void);
void Game_ProcessInput(void);
