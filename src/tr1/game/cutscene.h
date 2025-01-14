#pragma once

#include <libtrx/game/gameflow/types.h>

bool Cutscene_Start(int32_t level_num);
void Cutscene_End(void);
GAME_FLOW_COMMAND Cutscene_Control(void);
void Cutscene_Draw(void);
