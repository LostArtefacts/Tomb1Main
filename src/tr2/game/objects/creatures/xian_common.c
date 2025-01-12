#include "game/objects/creatures/xian_common.h"

#include "game/items.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "global/vars.h"

#include <libtrx/game/matrix.h>

// TODO: this duplicates Object_DrawAnimatingItem almost entirely
void XianWarrior_Draw(const ITEM *item)
{
    ANIM_FRAME *frames[2];
    int32_t rate;
    const int32_t frac = Item_GetFrames(item, frames, &rate);
    const OBJECT *const obj = Object_GetObject(item->object_id);

    if (obj->shadow_size != 0) {
        Output_InsertShadow(obj->shadow_size, &frames[0]->bounds, item);
    }

    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);

    const int32_t clip = Output_GetObjectBounds(&frames[0]->bounds);
    if (clip == 0) {
        Matrix_Pop();
        return;
    }

    Output_CalculateObjectLighting(item, &frames[0]->bounds);

    int16_t **normal_mesh_ptrs = NULL;
    int16_t **jade_mesh_ptrs = NULL;
    if (item->object_id == O_XIAN_SPEARMAN) {
        normal_mesh_ptrs = &g_Meshes[g_Objects[O_XIAN_SPEARMAN].mesh_idx];
        jade_mesh_ptrs = &g_Meshes[g_Objects[O_XIAN_SPEARMAN_STATUE].mesh_idx];
    } else {
        normal_mesh_ptrs = &g_Meshes[g_Objects[O_XIAN_KNIGHT].mesh_idx];
        jade_mesh_ptrs = &g_Meshes[g_Objects[O_XIAN_KNIGHT_STATUE].mesh_idx];
    }

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
                Output_InsertPolygons_I(normal_mesh_ptrs[mesh_idx], clip);
            } else {
                Output_InsertPolygons_I(jade_mesh_ptrs[mesh_idx], clip);
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
                Output_InsertPolygons(normal_mesh_ptrs[mesh_idx], clip);
            } else {
                Output_InsertPolygons(jade_mesh_ptrs[mesh_idx], clip);
            }
        }
    }

    Matrix_Pop();
}
