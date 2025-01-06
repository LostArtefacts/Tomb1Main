#pragma once

#include <libtrx/game/inventory_ring/types.h>
#include <libtrx/game/objects/types.h>
#include <libtrx/game/phase/types.h>

void InvRing_SetRequestedObjectID(GAME_OBJECT_ID object_id);

void InvRing_Construct(void);
void InvRing_Destroy(void);
PHASE_CONTROL InvRing_Close(INV_RING *ring, GAME_OBJECT_ID inv_chosen);
void InvRing_SelectMeshes(INVENTORY_ITEM *inv_item);
bool InvRing_AnimateItem(INVENTORY_ITEM *inv_item);
// TODO: make this return a GAME_FLOW_COMMAND
PHASE_CONTROL InvRing_Control(INV_RING *ring, int32_t num_frames);

void InvRing_InitHeader(INV_RING *ring);
void InvRing_RemoveHeader(void);
void InvRing_RemoveAllText(void);
void InvRing_Active(INVENTORY_ITEM *inv_item);
void InvRing_ResetItem(INVENTORY_ITEM *inv_item);

bool InvRing_CanExamine(void);
void InvRing_InitExamineOverlay(void);
void InvRing_RemoveExamineOverlay(void);
