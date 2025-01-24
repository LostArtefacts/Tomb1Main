#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <libtrx/game/game_flow/common.h>

void GF_Shutdown(void);

int32_t GF_GetCutsceneCount(void);
const char *GF_GetCutscenePath(int32_t cutscene_num);
const char *GF_GetTitleLevelPath(void);
START_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *level);
