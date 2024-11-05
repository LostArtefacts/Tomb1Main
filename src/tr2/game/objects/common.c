#include "game/objects/common.h"

#include "game/items.h"
#include "game/lara/misc.h"
#include "game/matrix.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

OBJECT *Object_GetObject(GAME_OBJECT_ID object_id)
{
    return &g_Objects[object_id];
}

void Object_DrawDummyItem(const ITEM *const item)
{
}

void __cdecl Object_Collision(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    ITEM *const item = &g_Items[item_num];

    if (!Item_TestBoundsCollide(item, lara_item, coll->radius)) {
        return;
    }

    if (!Collide_TestCollision(item, lara_item)) {
        return;
    }

    if (coll->enable_baddie_push) {
        Lara_Push(item, lara_item, coll, false, true);
    }
}

void __cdecl Object_Collision_Trap(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    ITEM *const item = &g_Items[item_num];

    if (item->status == IS_ACTIVE) {
        if (Item_TestBoundsCollide(item, lara_item, coll->radius)) {
            Collide_TestCollision(item, lara_item);
        }
    } else if (item->status != IS_INVISIBLE) {
        Object_Collision(item_num, lara_item, coll);
    }
}

BOUNDS_16 Object_GetBoundingBox(
    const OBJECT *const obj, const FRAME_INFO *const frame)
{
    int16_t **mesh_ptrs = &g_Meshes[obj->mesh_idx];
    int32_t *bone = &g_AnimBones[obj->bone_idx];
    const int16_t *mesh_rots = frame->mesh_rots;

    Matrix_PushUnit();
    Matrix_TranslateRel(frame->offset.x, frame->offset.y, frame->offset.z);
    Matrix_RotYXZsuperpack(&mesh_rots, 0);

    BOUNDS_16 new_bounds = {
        .min_x = 0x7FFF,
        .min_y = 0x7FFF,
        .min_z = 0x7FFF,
        .max_x = -0x7FFF,
        .max_y = -0x7FFF,
        .max_z = -0x7FFF,
    };

    for (int32_t mesh_idx = 0; mesh_idx < obj->mesh_count; mesh_idx++) {
        if (mesh_idx != 0) {
            int32_t bone_extra_flags = *bone;
            if (bone_extra_flags & BF_MATRIX_POP) {
                Matrix_Pop();
            }

            if (bone_extra_flags & BF_MATRIX_PUSH) {
                Matrix_Push();
            }

            Matrix_TranslateRel(bone[1], bone[2], bone[3]);
            Matrix_RotYXZsuperpack(&mesh_rots, 0);
            bone += 4;
        }

        const int16_t *obj_ptr = mesh_ptrs[mesh_idx];
        obj_ptr += 5;
        const int32_t vtx_count = *obj_ptr++;
        for (int32_t i = 0; i < vtx_count; i++) {
            // clang-format off
            const MATRIX *const mptr = g_MatrixPtr;
            const double xv = (
                mptr->_00 * obj_ptr[0] +
                mptr->_01 * obj_ptr[1] +
                mptr->_02 * obj_ptr[2] +
                mptr->_03
            );
            const double yv = (
                mptr->_10 * obj_ptr[0] +
                mptr->_11 * obj_ptr[1] +
                mptr->_12 * obj_ptr[2] +
                mptr->_13
            );
            double zv = (
                mptr->_20 * obj_ptr[0] +
                mptr->_21 * obj_ptr[1] +
                mptr->_22 * obj_ptr[2] +
                mptr->_23
            );
            // clang-format on

            const int32_t x = ((int32_t)xv) >> W2V_SHIFT;
            const int32_t y = ((int32_t)yv) >> W2V_SHIFT;
            const int32_t z = ((int32_t)zv) >> W2V_SHIFT;

            new_bounds.min_x = MIN(new_bounds.min_x, x);
            new_bounds.min_y = MIN(new_bounds.min_y, y);
            new_bounds.min_z = MIN(new_bounds.min_z, z);
            new_bounds.max_x = MAX(new_bounds.max_x, x);
            new_bounds.max_y = MAX(new_bounds.max_y, y);
            new_bounds.max_z = MAX(new_bounds.max_z, z);
            obj_ptr += 3;
        }
    }

    Matrix_Pop();
    return new_bounds;
}
