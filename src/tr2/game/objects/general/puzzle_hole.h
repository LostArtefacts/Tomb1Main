#pragma once

#include "global/types.h"

void PuzzleHole_Setup(OBJECT *obj, bool done);

void __cdecl PuzzleHole_Collision(
    int16_t item_num, ITEM *lara_item, COLL_INFO *coll);
