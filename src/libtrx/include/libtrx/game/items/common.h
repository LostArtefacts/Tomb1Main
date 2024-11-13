#pragma once

#include "./types.h"

#include <stdbool.h>

ITEM *Item_Get(int16_t num);
ITEM *Item_Find(GAME_OBJECT_ID object_id);
int32_t Item_GetTotalCount(void);
int32_t Item_GetDistance(const ITEM *item, const XYZ_32 *target);
void Item_TakeDamage(ITEM *item, int16_t damage, bool hit_status);
