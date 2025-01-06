#pragma once

#include "game/inventory_ring/types.h"
#include "game/phase/phase.h"

typedef struct {
    INVENTORY_MODE mode;
} PHASE_INVENTORY_ARGS;

extern PHASER g_InventoryPhaser;
