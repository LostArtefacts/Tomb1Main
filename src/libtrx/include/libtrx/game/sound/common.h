#pragma once

#include "../math.h"
#include "enum.h"
#include "ids.h"

void Sound_PauseAll(void);
void Sound_UnpauseAll(void);

extern void Sound_StopAll(void);
extern bool Sound_Effect(
    SOUND_EFFECT_ID sfx_num, const XYZ_32 *pos, uint32_t flags);
extern bool Sound_IsAvailable(SOUND_EFFECT_ID sfx_num);
