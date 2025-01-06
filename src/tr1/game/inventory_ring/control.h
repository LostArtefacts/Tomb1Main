#pragma once

#include <libtrx/game/gameflow/types.h>
#include <libtrx/game/inventory_ring/control.h>
#include <libtrx/game/inventory_ring/types.h>
#include <libtrx/game/objects/types.h>

INV_RING *InvRing_Open(INVENTORY_MODE mode);
void InvRing_Close(INV_RING *ring);

GAME_FLOW_COMMAND InvRing_Control(INV_RING *ring, int32_t num_frames);

void InvRing_InitHeader(INV_RING *ring);
void InvRing_RemoveHeader(void);
void InvRing_RemoveAllText(void);
void InvRing_ResetItem(INVENTORY_ITEM *inv_item);

bool InvRing_CanExamine(void);
