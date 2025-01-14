#pragma once

#include <libtrx/game/inventory_ring/control.h>

void InvRing_InitHeader(INV_RING *ring);
void InvRing_RemoveHeader(void);
void InvRing_RemoveAllText(void);
void InvRing_ResetItem(INVENTORY_ITEM *inv_item);

bool InvRing_CanExamine(void);
