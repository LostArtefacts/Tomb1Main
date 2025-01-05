#pragma once

#include "game/inventory_ring/types.h"

#include <libtrx/game/objects/types.h>
#include <libtrx/game/phase/types.h>

void InvRing_SetRequestedObjectID(GAME_OBJECT_ID object_id);

void InvRing_Construct(void);
void InvRing_Destroy(void);
PHASE_CONTROL InvRing_Close(GAME_OBJECT_ID inv_chosen);
void InvRing_SelectMeshes(INVENTORY_ITEM *inv_item);
bool InvRing_AnimateItem(INVENTORY_ITEM *inv_item);
// TODO: make this return a GAME_FLOW_COMMAND
PHASE_CONTROL InvRing_Control(
    RING_INFO *ring, IMOTION_INFO *motion, int32_t num_frames);

void InvRing_InitHeader(RING_INFO *ring);
void InvRing_RemoveHeader(void);
void InvRing_RemoveAllText(void);
void InvRing_Active(INVENTORY_ITEM *inv_item);
void InvRing_ResetItem(INVENTORY_ITEM *inv_item);

bool InvRing_CanExamine(void);
void InvRing_InitExamineOverlay(void);
void InvRing_RemoveExamineOverlay(void);
