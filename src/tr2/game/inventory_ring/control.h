#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/objects/types.h>

void InvRing_Init(void);

int32_t InvRing_Display(INVENTORY_MODE inventory_mode);
int32_t InvRing_DisplayKeys(GAME_OBJECT_ID receptacle_type_id);

void InvRing_ClearSelection(void);
