#pragma once

#include "global/types.h"

extern int16_t g_PickupBounds[];

void Pickup_Setup(OBJECT *obj);

void Pickup_Draw(const ITEM *item);

void Pickup_Collision(int16_t item_num, ITEM *lara_item, COLL_INFO *coll);

int32_t Pickup_Trigger(int16_t item_num);
