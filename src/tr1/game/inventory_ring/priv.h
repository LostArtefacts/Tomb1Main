#pragma once

#include "game/inventory_ring/types.h"

void InvRing_InitRing(
    RING_INFO *ring, int16_t type, INVENTORY_ITEM **list, int16_t qty,
    int16_t current, IMOTION_INFO *imo);

void InvRing_GetView(RING_INFO *ring, XYZ_32 *view_pos, XYZ_16 *view_rot);
void InvRing_Light(RING_INFO *ring);
void InvRing_CalcAdders(RING_INFO *ring, int16_t rotation_duration);
void InvRing_DoMotions(RING_INFO *ring);
void InvRing_RotateLeft(RING_INFO *ring);
void InvRing_RotateRight(RING_INFO *ring);
void InvRing_MotionInit(
    RING_INFO *ring, int16_t frames, int16_t status, int16_t status_target);
void InvRing_MotionSetup(
    RING_INFO *ring, int16_t status, int16_t status_target, int16_t frames);
void InvRing_MotionRadius(RING_INFO *ring, int16_t target);
void InvRing_MotionRotation(RING_INFO *ring, int16_t rotation, int16_t target);
void InvRing_MotionCameraPos(RING_INFO *ring, int16_t target);
void InvRing_MotionCameraPitch(RING_INFO *ring, int16_t target);
void InvRing_MotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item);
void InvRing_MotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item);
