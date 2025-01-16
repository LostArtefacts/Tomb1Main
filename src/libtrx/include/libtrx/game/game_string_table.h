#pragma once

#include <stdint.h>

bool GameStringTable_LoadFromFile(const char *path);
void GameStringTable_Apply(int32_t level_num);
void GameStringTable_Shutdown(void);
