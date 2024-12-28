#pragma once

#include "global/types.h"

void InvRing_SetRequestedObjectID(const GAME_OBJECT_ID object_id);
void InvRing_InitRing(
    RING_INFO *ring, RING_TYPE type, INVENTORY_ITEM **list, int16_t qty,
    int16_t current, IMOTION_INFO *imo);
void InvRing_GetView(const RING_INFO *ring, PHD_3DPOS *view);
void InvRing_Light(const RING_INFO *ring);
void InvRing_CalcAdders(RING_INFO *ring, int16_t rotation_duration);
void InvRing_DoMotions(RING_INFO *ring);
void InvRing_RotateLeft(RING_INFO *ring);
void InvRing_RotateRight(RING_INFO *ring);
void InvRing_MotionInit(
    RING_INFO *ring, int16_t frames, RING_STATUS status,
    RING_STATUS status_target);
void InvRing_MotionSetup(
    RING_INFO *ring, RING_STATUS status, RING_STATUS status_target,
    int16_t frames);
void InvRing_MotionRadius(RING_INFO *ring, int16_t target);
void InvRing_MotionRotation(RING_INFO *ring, int16_t rotation, int16_t target);
void InvRing_MotionCameraPos(RING_INFO *ring, int16_t target);
void InvRing_MotionCameraPitch(RING_INFO *ring, int16_t target);
void InvRing_MotionItemSelect(RING_INFO *ring, const INVENTORY_ITEM *inv_item);
void InvRing_MotionItemDeselect(
    RING_INFO *ring, const INVENTORY_ITEM *inv_item);
