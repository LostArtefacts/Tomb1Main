#pragma once

#include "game/inventory_ring/types.h"
#include "global/types.h"

void InvRing_SetRequestedObjectID(const GAME_OBJECT_ID object_id);
void InvRing_InitRing(
    INV_RING *ring, RING_TYPE type, INV_ITEM **list, int16_t qty,
    int16_t current);
void InvRing_GetView(const INV_RING *ring, PHD_3DPOS *view);
void InvRing_Light(const INV_RING *ring);
void InvRing_CalcAdders(INV_RING *ring, int16_t rotation_duration);
void InvRing_DoMotions(INV_RING *ring);
void InvRing_RotateLeft(INV_RING *ring);
void InvRing_RotateRight(INV_RING *ring);
void InvRing_MotionInit(
    INV_RING *ring, int16_t frames, RING_STATUS status,
    RING_STATUS status_target);
void InvRing_MotionSetup(
    INV_RING *ring, RING_STATUS status, RING_STATUS status_target,
    int16_t frames);
void InvRing_MotionRadius(INV_RING *ring, int16_t target);
void InvRing_MotionRotation(INV_RING *ring, int16_t rotation, int16_t target);
void InvRing_MotionCameraPos(INV_RING *ring, int16_t target);
void InvRing_MotionCameraPitch(INV_RING *ring, int16_t target);
void InvRing_MotionItemSelect(INV_RING *ring, const INV_ITEM *inv_item);
void InvRing_MotionItemDeselect(INV_RING *ring, const INV_ITEM *inv_item);
