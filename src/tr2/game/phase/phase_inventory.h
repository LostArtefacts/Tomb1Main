#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/phase.h>

PHASE *Phase_Inventory_Create(INVENTORY_MODE mode);
void Phase_Inventory_Destroy(PHASE *phase);
