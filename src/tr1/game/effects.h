#pragma once

#include "global/types.h"

#include <stdint.h>

extern EFFECT *g_Effects;
extern int16_t g_NextFxActive;

void Effect_InitialiseArray(void);
void Effect_Control(void);
int16_t Effect_Create(int16_t room_num);
void Effect_Kill(int16_t effect_num);
void Effect_NewRoom(int16_t effect_num, int16_t room_num);
void Effect_Draw(int16_t fxnum);
void Effect_RunActiveFlipEffect(void);
