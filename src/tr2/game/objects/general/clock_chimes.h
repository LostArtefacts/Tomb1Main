#pragma once

#include "global/types.h"

void ClockChimes_Setup(void);
void __cdecl DoChimeSound(const ITEM *item);
void __cdecl ClockChimes_Control(int16_t item_num);
