#pragma once

#include <libtrx/game/inventory_ring/types.h>
#include <libtrx/game/phase/types.h>

PHASE *Phase_Inventory_Create(INVENTORY_MODE mode);
void Phase_Inventory_Destroy(PHASE *phase);
