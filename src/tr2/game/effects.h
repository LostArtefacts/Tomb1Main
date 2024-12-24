#pragma once

#include <libtrx/game/effects/types.h>

#define NO_EFFECT (-1)

void __cdecl Effect_InitialiseArray(void);
EFFECT *Effect_Get(int16_t effect_num);
int16_t Effect_GetActiveNum(void);
int16_t __cdecl Effect_Create(int16_t room_num);
void __cdecl Effect_Kill(int16_t effect_num);
void __cdecl Effect_NewRoom(int16_t effect_num, int16_t room_num);
void __cdecl Effect_Draw(int16_t effect_num);
