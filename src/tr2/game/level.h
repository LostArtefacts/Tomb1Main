#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

bool Level_Initialise(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);
bool Level_Load(const GAME_FLOW_LEVEL *level);
void Level_Unload(void);
