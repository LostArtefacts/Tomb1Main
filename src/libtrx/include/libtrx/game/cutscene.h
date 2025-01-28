#pragma once

#include "./game_flow/types.h"

bool Cutscene_Start(int32_t level_num);
void Cutscene_End(void);
GF_COMMAND Cutscene_Control(void);
void Cutscene_Draw(void);
