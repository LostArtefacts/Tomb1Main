#pragma once

#include "global/types.h"

void SphereOfDoom_Setup(OBJECT *obj, bool transparent);
void __cdecl SphereOfDoom_Collision(
    int16_t item_num, ITEM *lara_item, COLL_INFO *coll);
