#pragma once

#include "global/types.h"

bool Level_Initialise(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
bool Level_Load(const char *file_name, int32_t level_num);
void Level_Unload(void);
