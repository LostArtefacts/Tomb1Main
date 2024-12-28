#pragma once

#include "global/types.h"

void Inv_InsertItem(INVENTORY_ITEM *inv_item);
int32_t Inv_AddItem(GAME_OBJECT_ID object_id);
void Inv_AddItemNTimes(GAME_OBJECT_ID object_id, int32_t qty);
int32_t Inv_RequestItem(GAME_OBJECT_ID object_id);
void Inv_RemoveAllItems(void);
int32_t Inv_RemoveItem(GAME_OBJECT_ID object_id);
GAME_OBJECT_ID Inv_GetItemOption(GAME_OBJECT_ID object_id);
