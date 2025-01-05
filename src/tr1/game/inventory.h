#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/inventory.h>

void Inv_InsertItem(INVENTORY_ITEM *inv_item);
int32_t Inv_RequestItem(GAME_OBJECT_ID object_id);
void Inv_RemoveAllItems(void);
void Inv_ClearSelection(void);
bool Inv_RemoveItem(GAME_OBJECT_ID object_id);
GAME_OBJECT_ID Inv_GetItemOption(GAME_OBJECT_ID object_id);
