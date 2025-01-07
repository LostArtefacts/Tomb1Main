#pragma once

#include "../math/types.h"
#include "./types.h"

#define INV_RING_OPEN_ROTATION DEG_180
#define INV_RING_ROTATE_DURATION 24
#define INV_RING_OPEN_FRAMES 32
#define INV_RING_CAMERA_HEIGHT (-0x100) // = -256
#define INV_RING_CAMERA_START_HEIGHT (-0x600) // = -1536
#define INV_RING_RADIUS 688

void InvRing_InitRing(
    INV_RING *ring, RING_TYPE type, INVENTORY_ITEM **list, int16_t qty,
    int16_t current);
void InvRing_InitInvItem(INVENTORY_ITEM *inv_item);

void InvRing_GetView(const INV_RING *ring, XYZ_32 *out_pos, XYZ_16 *out_rot);
void InvRing_Light(const INV_RING *ring);
void InvRing_CalcAdders(INV_RING *ring, int16_t rotation_duration);
void InvRing_DoMotions(INV_RING *ring);
void InvRing_RotateLeft(INV_RING *ring);
void InvRing_RotateRight(INV_RING *ring);

void InvRing_MotionInit(
    INV_RING *ring, RING_STATUS status, RING_STATUS status_target,
    int16_t frames);
void InvRing_MotionSetup(
    INV_RING *ring, RING_STATUS status, RING_STATUS status_target,
    int16_t frames);
void InvRing_MotionRadius(INV_RING *ring, int16_t target);
void InvRing_MotionRotation(INV_RING *ring, int16_t rotation, int16_t target);
void InvRing_MotionCameraPos(INV_RING *ring, int16_t target);
void InvRing_MotionCameraPitch(INV_RING *ring, int16_t target);
void InvRing_MotionItemSelect(INV_RING *ring, const INVENTORY_ITEM *inv_item);
void InvRing_MotionItemDeselect(INV_RING *ring, const INVENTORY_ITEM *inv_item);
