#pragma once

#include "game/phase/common.h"
#include "global/types.h"

PHASE *Phase_Game_Create(int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
void Phase_Game_Destroy(PHASE *phase);
