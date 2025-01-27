#pragma once

#include "../game_flow/types.h"
#include "./types.h"

PHASE *Phase_Game_Create(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);
void Phase_Game_Destroy(PHASE *phase);
