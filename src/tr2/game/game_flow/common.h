#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <libtrx/game/game_flow/common.h>

void GF_Shutdown(void);

int32_t GF_GetCutsceneCount(void);
void GF_SetCurrentLevel(GAME_FLOW_LEVEL *level);
START_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *level);
