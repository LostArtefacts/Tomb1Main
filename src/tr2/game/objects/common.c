#include "game/objects/common.h"

#include "game/collide.h"
#include "game/items.h"
#include "game/lara/misc.h"
#include "game/output.h"
#include "game/room.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/game/matrix.h>
#include <libtrx/utils.h>

OBJECT *Object_GetObject(GAME_OBJECT_ID object_id)
{
    return &g_Objects[object_id];
}

void Object_DrawDummyItem(const ITEM *const item)
{
}

void Object_DrawAnimatingItem(const ITEM *item)
{
    ANIM_FRAME *frames[2];
    int32_t rate;
    int32_t frac = Item_GetFrames(item, frames, &rate);
    const OBJECT *const obj = Object_GetObject(item->object_id);

    if (obj->shadow_size != 0) {
        Output_InsertShadow(obj->shadow_size, &frames[0]->bounds, item);
    }

    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);

    const int32_t clip = Output_GetObjectBounds(&frames[0]->bounds);
    if (!clip) {
        Matrix_Pop();
        return;
    }

    Output_CalculateObjectLighting(item, &frames[0]->bounds);

    int16_t *const *mesh_ptrs = &g_Meshes[obj->mesh_idx];
    const int16_t *extra_rotation = item->data;

    if (frac != 0) {
        for (int32_t mesh_idx = 0; mesh_idx < obj->mesh_count; mesh_idx++) {
            if (mesh_idx == 0) {
                Matrix_InitInterpolate(frac, rate);
                Matrix_TranslateRel_ID(
                    frames[0]->offset.x, frames[0]->offset.y,
                    frames[0]->offset.z, frames[1]->offset.x,
                    frames[1]->offset.y, frames[1]->offset.z);
                Matrix_RotXYZ16_I(
                    frames[0]->mesh_rots[mesh_idx],
                    frames[1]->mesh_rots[mesh_idx]);
            } else {
                const ANIM_BONE *const bone = Object_GetBone(obj, mesh_idx - 1);
                if (bone->matrix_pop) {
                    Matrix_Pop_I();
                }
                if (bone->matrix_push) {
                    Matrix_Push_I();
                }

                Matrix_TranslateRel_I(bone->pos.x, bone->pos.y, bone->pos.z);
                Matrix_RotXYZ16_I(
                    frames[0]->mesh_rots[mesh_idx],
                    frames[1]->mesh_rots[mesh_idx]);
                if (extra_rotation != NULL) {
                    if (bone->rot_y) {
                        Matrix_RotY_I(*extra_rotation++);
                    }
                    if (bone->rot_x) {
                        Matrix_RotX_I(*extra_rotation++);
                    }
                    if (bone->rot_z) {
                        Matrix_RotZ_I(*extra_rotation++);
                    }
                }
            }

            if (item->mesh_bits & (1 << mesh_idx)) {
                Output_InsertPolygons_I(mesh_ptrs[mesh_idx], clip);
            }
        }
    } else {
        for (int32_t mesh_idx = 0; mesh_idx < obj->mesh_count; mesh_idx++) {
            if (mesh_idx == 0) {
                Matrix_TranslateRel(
                    frames[0]->offset.x, frames[0]->offset.y,
                    frames[0]->offset.z);
                Matrix_RotXYZ16(frames[0]->mesh_rots[mesh_idx]);
            } else {
                const ANIM_BONE *const bone = Object_GetBone(obj, mesh_idx - 1);
                if (bone->matrix_pop) {
                    Matrix_Pop();
                }
                if (bone->matrix_push) {
                    Matrix_Push();
                }

                Matrix_TranslateRel(bone->pos.x, bone->pos.y, bone->pos.z);
                Matrix_RotXYZ16(frames[0]->mesh_rots[mesh_idx]);
                if (extra_rotation != NULL) {
                    if (bone->rot_y) {
                        Matrix_RotY(*extra_rotation++);
                    }
                    if (bone->rot_x) {
                        Matrix_RotX(*extra_rotation++);
                    }
                    if (bone->rot_z) {
                        Matrix_RotZ(*extra_rotation++);
                    }
                }
            }

            if (item->mesh_bits & (1 << mesh_idx)) {
                Output_InsertPolygons(mesh_ptrs[mesh_idx], clip);
            }
        }
    }

    Matrix_Pop();
}

void Object_DrawUnclippedItem(const ITEM *const item)
{
    const VIEWPORT old_vp = *Viewport_Get();

    VIEWPORT new_vp = old_vp;
    new_vp.game_vars.win_top = 0;
    new_vp.game_vars.win_left = 0;
    new_vp.game_vars.win_bottom = new_vp.game_vars.win_max_y;
    new_vp.game_vars.win_right = new_vp.game_vars.win_max_x;

    Viewport_Restore(&new_vp);
    Object_DrawAnimatingItem(item);
    Viewport_Restore(&old_vp);
}

void Object_DrawSpriteItem(const ITEM *const item)
{
    Output_CalculateStaticMeshLight(
        item->pos.x, item->pos.y, item->pos.z, item->shade_1, item->shade_2,
        Room_Get(item->room_num));

    const OBJECT *const obj = Object_GetObject(item->object_id);

    Output_DrawSprite(
        SPRITE_ABS | (obj->semi_transparent ? SPRITE_SEMI_TRANS : 0)
            | SPRITE_SHADE,
        item->pos.x, item->pos.y, item->pos.z, obj->mesh_idx - item->frame_num,
        g_LsAdder + 4096, 0);
}

void Object_Collision(
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

void Object_Collision_Trap(
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
    const OBJECT *const obj, const ANIM_FRAME *const frame,
    const uint32_t mesh_bits)
{
    int16_t **mesh_ptrs = &g_Meshes[obj->mesh_idx];
    const XYZ_16 *const mesh_rots = frame != NULL ? frame->mesh_rots : NULL;

    Matrix_PushUnit();
    if (frame != NULL) {
        Matrix_TranslateRel(frame->offset.x, frame->offset.y, frame->offset.z);
    }
    if (mesh_rots != NULL) {
        Matrix_RotXYZ16(mesh_rots[0]);
    }

    BOUNDS_16 new_bounds = {
        .min.x = 0x7FFF,
        .min.y = 0x7FFF,
        .min.z = 0x7FFF,
        .max.x = -0x7FFF,
        .max.y = -0x7FFF,
        .max.z = -0x7FFF,
    };

    for (int32_t mesh_idx = 0; mesh_idx < obj->mesh_count; mesh_idx++) {
        if (mesh_idx != 0) {
            const ANIM_BONE *const bone = Object_GetBone(obj, mesh_idx - 1);
            if (bone->matrix_pop) {
                Matrix_Pop();
            }

            if (bone->matrix_push) {
                Matrix_Push();
            }

            Matrix_TranslateRel(bone->pos.x, bone->pos.y, bone->pos.z);
            if (mesh_rots != NULL) {
                Matrix_RotXYZ16(mesh_rots[mesh_idx]);
            }
        }

        if (!(mesh_bits & (1 << mesh_idx))) {
            continue;
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

            new_bounds.min.x = MIN(new_bounds.min.x, x);
            new_bounds.min.y = MIN(new_bounds.min.y, y);
            new_bounds.min.z = MIN(new_bounds.min.z, z);
            new_bounds.max.x = MAX(new_bounds.max.x, x);
            new_bounds.max.y = MAX(new_bounds.max.y, y);
            new_bounds.max.z = MAX(new_bounds.max.z, z);
            obj_ptr += 3;
        }
    }

    Matrix_Pop();
    return new_bounds;
}
