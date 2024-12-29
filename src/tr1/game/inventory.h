#pragma once

#include "global/types.h"

#include <libtrx/game/inventory.h>

#include <stdbool.h>
#include <stdint.h>

bool Inv_Display(INV_MODE inv_mode);
bool Inv_DisplayKeys(GAME_OBJECT_ID receptacle_type_id);

void Inv_InsertItem(INVENTORY_ITEM *inv_item);
int32_t Inv_RequestItem(GAME_OBJECT_ID object_id);
void Inv_RemoveAllItems(void);
void Inv_ClearSelection(void);
bool Inv_RemoveItem(GAME_OBJECT_ID object_id);
GAME_OBJECT_ID Inv_GetItemOption(GAME_OBJECT_ID object_id);
