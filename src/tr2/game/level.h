#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

bool Level_Initialise(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
bool Level_Load(const GAME_FLOW_LEVEL *level);
void Level_Unload(void);
