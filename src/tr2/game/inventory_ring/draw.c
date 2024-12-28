#include "game/inventory_ring/draw.h"

#include "decomp/savegame.h"
#include "game/console/common.h"
#include "game/input.h"
#include "game/inventory_ring/ring.h"
#include "game/matrix.h"
#include "game/option/option.h"
#include "game/output.h"
#include "game/overlay.h"
#include "global/vars.h"

static void M_DrawItem(const INV_RING *ring, const INV_ITEM *inv_item);

static void M_DrawItem(
    const INV_RING *const ring, const INV_ITEM *const inv_item)
{
    if (ring->motion.status == RNG_DONE) {
        g_LsAdder = LOW_LIGHT;
    } else if (inv_item != ring->list[ring->current_object]) {
        g_LsAdder = LOW_LIGHT;
    } else if (ring->rotating) {
        g_LsAdder = LOW_LIGHT;
    } else {
        g_LsAdder = HIGH_LIGHT;
    }

    int32_t minutes;
    int32_t hours;
    int32_t seconds;
    if (inv_item->object_id == O_COMPASS_OPTION) {
        const int32_t total_seconds =
            g_SaveGame.statistics.timer / FRAMES_PER_SECOND;
        hours = (total_seconds % 43200) * PHD_DEGREE * -360 / 43200;
        minutes = (total_seconds % 3600) * PHD_DEGREE * -360 / 3600;
        seconds = (total_seconds % 60) * PHD_DEGREE * -360 / 60;
    } else {
        seconds = 0;
        minutes = 0;
        hours = 0;
    }

    Matrix_TranslateRel(0, inv_item->y_trans, inv_item->z_trans);
    Matrix_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);
    const OBJECT *const obj = &g_Objects[inv_item->object_id];
    if ((obj->flags & 1) == 0) {
        return;
    }

    if (obj->mesh_count < 0) {
        Output_DrawSprite(0, 0, 0, 0, obj->mesh_idx, 0, 0);
        return;
    }

    if (inv_item->sprite_list != NULL) {
        const int32_t zv = g_MatrixPtr->_23;
        const int32_t zp = zv / g_PhdPersp;
        const int32_t sx = g_PhdWinCenterX + g_MatrixPtr->_03 / zp;
        const int32_t sy = g_PhdWinCenterY + g_MatrixPtr->_13 / zp;

        INVENTORY_SPRITE **sprite_list = inv_item->sprite_list;
        INVENTORY_SPRITE *sprite;
        while ((sprite = *sprite_list++)) {
            if (zv < g_PhdNearZ || zv > g_PhdFarZ) {
                break;
            }

            while (sprite->shape) {
                switch (sprite->shape) {
                case SHAPE_SPRITE:
                    Output_DrawScreenSprite(
                        sx + sprite->pos.x, sy + sprite->pos.y, sprite->pos.z,
                        sprite->param1, sprite->param2,
                        g_Objects[O_ALPHABET].mesh_idx + sprite->sprite_num,
                        4096, 0);
                    break;

                case SHAPE_LINE:
                    Output_DrawScreenLine(
                        sx + sprite->pos.x, sy + sprite->pos.y, sprite->pos.z,
                        sprite->param1, sprite->param2, sprite->sprite_num,
                        sprite->grdptr, 0);
                    break;

                case SHAPE_BOX:
                    Output_DrawScreenBox(
                        sx + sprite->pos.x, sy + sprite->pos.y, sprite->pos.z,
                        sprite->param1, sprite->param2, sprite->sprite_num,
                        sprite->grdptr, 0);
                    break;

                case SHAPE_FBOX:
                    Output_DrawScreenFBox(
                        sx + sprite->pos.x, sy + sprite->pos.y, sprite->pos.z,
                        sprite->param1, sprite->param2, sprite->sprite_num,
                        sprite->grdptr, 0);
                    break;

                default:
                    break;
                }
                sprite++;
            }
        }
    }

    FRAME_INFO *frame_ptr = (FRAME_INFO *)&obj->frame_base
                                [inv_item->current_frame
                                 * (g_Anims[obj->anim_idx].interpolation >> 8)];

    Matrix_Push();
    const int32_t clip = Output_GetObjectBounds(&frame_ptr->bounds);
    if (!clip) {
        Matrix_Pop();
        return;
    }

    const int32_t *bone = &g_AnimBones[obj->bone_idx];
    Matrix_TranslateRel(
        frame_ptr->offset.x, frame_ptr->offset.y, frame_ptr->offset.z);
    const int16_t *rot = frame_ptr->mesh_rots;
    Matrix_RotYXZsuperpack(&rot, 0);

    for (int32_t mesh_idx = 0; mesh_idx < obj->mesh_count; mesh_idx++) {
        if (mesh_idx > 0) {
            const int32_t bone_flags = bone[0];
            if (bone_flags & BF_MATRIX_POP) {
                Matrix_Pop();
            }
            if (bone_flags & BF_MATRIX_PUSH) {
                Matrix_Push();
            }

            Matrix_TranslateRel(bone[1], bone[2], bone[3]);
            Matrix_RotYXZsuperpack(&rot, 0);
            bone += 4;

            if (inv_item->object_id == O_COMPASS_OPTION) {
                if (mesh_idx == 6) {
                    Matrix_RotZ(seconds);
#if 0
                    const int32_t tmp = inv_item->reserved[0];
                    inv_item->reserved[0] = seconds;
                    inv_item->reserved[1] = tmp;
#endif
                }
                if (mesh_idx == 5) {
                    Matrix_RotZ(minutes);
                }
                if (mesh_idx == 4) {
                    Matrix_RotZ(hours);
                }
            }
        }

        if (inv_item->meshes_drawn & (1 << mesh_idx)) {
            Output_InsertPolygons(g_Meshes[obj->mesh_idx + mesh_idx], clip);
        }
    }

    Matrix_Pop();
}

void InvRing_Draw(INV_RING *const ring)
{
    ring->camera.pos.z = ring->radius + 598;

    Output_DrawBackground();

    PHD_3DPOS view;
    InvRing_GetView(ring, &view);
    Matrix_GenerateW2V(&view);
    InvRing_Light(ring);

    Matrix_Push();
    Matrix_TranslateAbs(
        ring->ring_pos.pos.x, ring->ring_pos.pos.y, ring->ring_pos.pos.z);
    Matrix_RotYXZ(
        ring->ring_pos.rot.y, ring->ring_pos.rot.x, ring->ring_pos.rot.z);

    int32_t angle = 0;
    for (int32_t i = 0; i < ring->number_of_objects; i++) {
        INV_ITEM *const inv_item = ring->list[i];
        Matrix_Push();
        Matrix_RotYXZ(angle, 0, 0);
        Matrix_TranslateRel(ring->radius, 0, 0);
        Matrix_RotYXZ(PHD_90, inv_item->x_rot_pt, 0);
        M_DrawItem(ring, inv_item);
        angle += ring->angle_adder;
        Matrix_Pop();
    }

    if (ring->list != NULL && !ring->rotating
        && (ring->motion.status == RNG_OPEN
            || ring->motion.status == RNG_SELECTING
            || ring->motion.status == RNG_SELECTED
            || ring->motion.status == RNG_DESELECTING
            || ring->motion.status == RNG_DESELECT
            || ring->motion.status == RNG_CLOSING_ITEM)) {
        const INV_ITEM *const inv_item = ring->list[ring->current_object];
        if (inv_item != NULL) {
            switch (inv_item->object_id) {
            case O_SMALL_MEDIPACK_OPTION:
            case O_LARGE_MEDIPACK_OPTION:
                Overlay_DrawHealthBar();
                break;

            default:
                break;
            }
        }
    }

    Matrix_Pop();
    Output_DrawPolyList();

    if (ring->motion.status == RNG_SELECTED) {
        INV_ITEM *const inv_item = ring->list[ring->current_object];
        if (inv_item->object_id == O_PASSPORT_CLOSED) {
            inv_item->object_id = O_PASSPORT_OPTION;
        }
        Option_Draw(inv_item);
    }

    Overlay_DrawModeInfo();
    Text_Draw();
    Output_DrawPolyList();

#if 0
    Output_DrawBlackRectangle(Fader_GetCurrentValue(fader));
#endif
    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
}
