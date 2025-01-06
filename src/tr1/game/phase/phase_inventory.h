#pragma once

#include "game/phase/phase.h"

#include <libtrx/game/inventory_ring/types.h>

typedef struct {
    INVENTORY_MODE mode;
} PHASE_INVENTORY_ARGS;

extern PHASER g_InventoryPhaser;
