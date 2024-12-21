#pragma once

#include "global/types.h"

void Spikes_Setup(void);
void __cdecl Spikes_Collision(
    int16_t item_num, ITEM *lara_item, COLL_INFO *coll);
