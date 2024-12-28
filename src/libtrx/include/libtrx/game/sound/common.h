#pragma once

#include "../math.h"
#include "enum.h"
#include "ids.h"

bool Sound_Effect(SOUND_EFFECT_ID sfx_num, const XYZ_32 *pos, uint32_t flags);
bool Sound_IsAvailable(SOUND_EFFECT_ID sfx_num);
