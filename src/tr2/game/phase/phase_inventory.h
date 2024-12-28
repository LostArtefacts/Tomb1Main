#pragma once

#include "game/inventory_ring/types.h"
#include "game/phase/common.h"

PHASE *Phase_Inventory_Create(INVENTORY_MODE mode);
void Phase_Inventory_Destroy(PHASE *phase);
