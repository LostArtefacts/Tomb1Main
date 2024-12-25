#pragma once

#include "global/types.h"

void Collide_GetCollisionInfo(
    COLL_INFO *coll, int32_t x_pos, int32_t y_pos, int32_t z_pos,
    int16_t room_num, int32_t obj_height);

int32_t Collide_CollideStaticObjects(
    COLL_INFO *coll, int32_t x, int32_t y, int32_t z, int16_t room_num,
    int32_t height);

int32_t Collide_TestCollision(ITEM *item, const ITEM *lara_item);

int32_t Collide_GetSpheres(const ITEM *item, SPHERE *spheres, bool world_space);

void Collide_GetJointAbsPosition(
    const ITEM *item, XYZ_32 *out_vec, int32_t joint);
