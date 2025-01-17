#pragma once

#include "./game_flow/types.h"

bool Demo_Start(int32_t level_num);
void Demo_End(void);
void Demo_Pause(void);
void Demo_Unpause(void);
void Demo_StopFlashing(void);

bool Demo_GetInput(void);
GAME_FLOW_COMMAND Demo_Control(void);
int32_t Demo_ChooseLevel(int32_t demo_num);
