#pragma once

#include <libtrx/game/effects/types.h>

#define NO_EFFECT (-1)

void Effect_InitialiseArray(void);
EFFECT *Effect_Get(int16_t effect_num);
int16_t Effect_GetNum(const EFFECT *effect);
int16_t Effect_GetActiveNum(void);
void Effect_Control(void);
int16_t Effect_Create(int16_t room_num);
void Effect_Kill(int16_t effect_num);
void Effect_NewRoom(int16_t effect_num, int16_t room_num);
void Effect_Draw(int16_t effect_num);
void Effect_RunActiveFlipEffect(void);
