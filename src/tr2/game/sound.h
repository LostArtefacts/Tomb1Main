#pragma once

#include "global/types.h"

#include <libtrx/game/sound.h>

#define SOUND_DEFAULT_PITCH 0x10000

void Sound_Init(void);
void Sound_Shutdown(void);

void Sound_SetMasterVolume(int32_t volume);
void Sound_UpdateEffects(void);
void Sound_StopEffect(SOUND_EFFECT_ID sample_id);
void Sound_EndScene(void);
int32_t Sound_GetMinVolume(void);
int32_t Sound_GetMaxVolume(void);
