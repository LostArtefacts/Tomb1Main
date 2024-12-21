#pragma once

#include "global/types.h"

void TeethTrap_Setup(void);
void __cdecl TeethTrap_Bite(ITEM *item, const BITE *bite);
void __cdecl TeethTrap_Control(int16_t item_num);
