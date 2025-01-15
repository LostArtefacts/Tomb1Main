#pragma once

#include "../gameflow/types.h"
#include "./types.h"

PHASE *Phase_Game_Create(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
void Phase_Game_Destroy(PHASE *phase);
