#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <libtrx/game/game_flow/common.h>

void GF_Shutdown(void);
RESUME_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *level);
