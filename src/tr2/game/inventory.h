#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/inventory.h>
#include <libtrx/game/objects/types.h>

void Inv_InsertItem(INV_ITEM *inv_item);
int32_t Inv_RequestItem(GAME_OBJECT_ID object_id);
void Inv_RemoveAllItems(void);
int32_t Inv_RemoveItem(GAME_OBJECT_ID object_id);
GAME_OBJECT_ID Inv_GetItemOption(GAME_OBJECT_ID object_id);
