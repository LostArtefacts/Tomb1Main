#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <libtrx/game/game_flow/common.h>

void GF_Shutdown(void);

int32_t GF_GetCutsceneCount(void);
GAME_FLOW_LEVEL *GF_GetLevel(int32_t num, GAME_FLOW_LEVEL_TYPE level_type);
START_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *level);
