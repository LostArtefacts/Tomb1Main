#pragma once

#include "../game_flow/types.h"
#include "./types.h"

PHASE *Phase_Game_Create(const GF_LEVEL *level, GF_SEQUENCE_CONTEXT seq_ctx);
void Phase_Game_Destroy(PHASE *phase);
