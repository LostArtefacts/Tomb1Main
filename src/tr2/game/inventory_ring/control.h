#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/objects/types.h>

INV_RING *InvRing_Open(INVENTORY_MODE mode);
GAME_FLOW_COMMAND InvRing_Control(INV_RING *ring, int32_t num_frames);
GAME_FLOW_COMMAND InvRing_Close(INV_RING *ring);

void InvRing_ClearSelection(void);
void InvRing_SetRequestedObjectID(const GAME_OBJECT_ID object_id);
