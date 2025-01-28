#pragma once

#include "../game_flow/types.h"
#include "../objects/types.h"
#include "./types.h"

INV_RING *InvRing_Open(INVENTORY_MODE mode);
void InvRing_Close(INV_RING *ring);

GF_COMMAND InvRing_Control(INV_RING *ring, int32_t num_frames);

void InvRing_SetRequestedObjectID(GAME_OBJECT_ID object_id);
