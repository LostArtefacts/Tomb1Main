#pragma once

#include "global/types.h"

#include <libtrx/game/objects/common.h>

void Object_DrawDummyItem(const ITEM *item);
void __cdecl Object_DrawAnimatingItem(const ITEM *item);
void __cdecl Object_DrawUnclippedItem(const ITEM *item);
void __cdecl Object_DrawSpriteItem(const ITEM *item);

void __cdecl Object_Collision(
    int16_t item_num, ITEM *lara_item, COLL_INFO *coll);

void __cdecl Object_Collision_Trap(
    int16_t item_num, ITEM *lara_item, COLL_INFO *coll);

BOUNDS_16 Object_GetBoundingBox(
    const OBJECT *obj, const FRAME_INFO *frame, uint32_t mesh_bits);
