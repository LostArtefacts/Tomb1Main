#pragma once

#include <libtrx/game/phase.h>

#include <stdint.h>

PHASE *Phase_Cutscene_Create(int32_t level_num);
void Phase_Cutscene_Destroy(PHASE *phase);
