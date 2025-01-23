#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <stdint.h>

void Level_Load(const GAME_FLOW_LEVEL *level);
bool Level_Initialise(const GAME_FLOW_LEVEL *level);
const LEVEL_INFO *Level_GetInfo(void);
