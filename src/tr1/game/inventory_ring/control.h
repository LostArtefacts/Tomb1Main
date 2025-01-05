#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/objects/types.h>

void InvRing_SetRequestedObjectID(GAME_OBJECT_ID object_id);

void InvRing_InitHeader(RING_INFO *ring);
void InvRing_RemoveHeader(void);
void InvRing_RemoveAllText(void);
void InvRing_Active(INVENTORY_ITEM *inv_item);
void InvRing_ResetItem(INVENTORY_ITEM *inv_item);

bool InvRing_CanExamine(void);
void InvRing_InitExamineOverlay(void);
void InvRing_RemoveExamineOverlay(void);
