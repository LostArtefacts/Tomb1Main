#pragma once

#include <libtrx/game/inventory_ring/control.h>
#include <libtrx/game/inventory_ring/types.h>
#include <libtrx/game/objects/types.h>
#include <libtrx/game/phase/types.h>

INV_RING *InvRing_Open(INVENTORY_MODE mode);
PHASE_CONTROL InvRing_Close(INV_RING *ring);
// TODO: make this return a GAME_FLOW_COMMAND
PHASE_CONTROL InvRing_Control(INV_RING *ring, int32_t num_frames);

void InvRing_InitHeader(INV_RING *ring);
void InvRing_RemoveHeader(void);
void InvRing_RemoveAllText(void);
void InvRing_ResetItem(INVENTORY_ITEM *inv_item);

bool InvRing_CanExamine(void);
