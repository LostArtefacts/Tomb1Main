#pragma once

#include "global/types.h"

void InvRing_Init(void);

int32_t InvRing_Display(INVENTORY_MODE inventory_mode);
int32_t InvRing_DisplayKeys(GAME_OBJECT_ID receptacle_type_id);

void InvRing_ClearSelection(void);
