#include "game/inventory_ring/draw.h"

#include "game/game.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/inventory_ring.h"
#include "game/inventory_ring/control.h"
#include "game/objects/common.h"
#include "game/option.h"
#include "game/option/option_compass.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/shell.h"
#include "game/text.h"
#include "game/viewport.h"
#include "global/vars.h"
#include "math/matrix.h"

#include <libtrx/config.h>

CAMERA_INFO g_InvRing_OldCamera;
CLOCK_TIMER g_InvRing_MotionTimer = { 0 };

static int32_t M_GetFrames(
    const RING_INFO *ring, const IMOTION_INFO *motion,
    const INVENTORY_ITEM *inv_item, ANIM_FRAME **out_frame1,
    ANIM_FRAME **out_frame2, int32_t *out_rate);
static void M_DrawItem(
    const RING_INFO *ring, const IMOTION_INFO *motion, INVENTORY_ITEM *inv_item,
    int32_t num_frames);

static int32_t M_GetFrames(
    const RING_INFO *const ring, const IMOTION_INFO *const motion,
    const INVENTORY_ITEM *const inv_item, ANIM_FRAME **const out_frame1,
    ANIM_FRAME **const out_frame2, int32_t *const out_rate)
{
    const OBJECT *const obj = Object_GetObject(inv_item->object_id);
    const INVENTORY_ITEM *const cur_inv_item = ring->list[ring->current_object];
    if (inv_item != cur_inv_item
        || (motion->status != RNG_SELECTED
            && motion->status != RNG_CLOSING_ITEM)) {
        // only apply to animations, eg. the states where Inv_AnimateItem is
        // being actively called
        goto fallback;
    }

    if (inv_item->current_frame == inv_item->goal_frame
        || inv_item->frames_total == 1 || g_Config.rendering.fps == 30) {
        goto fallback;
    }

    const int32_t cur_frame_num = inv_item->current_frame;
    int32_t next_frame_num = inv_item->current_frame + inv_item->anim_direction;
    if (next_frame_num < 0) {
        next_frame_num = 0;
    }
    if (next_frame_num >= inv_item->frames_total) {
        next_frame_num = 0;
    }

    *out_frame1 = &obj->frame_base[cur_frame_num];
    *out_frame2 = &obj->frame_base[next_frame_num];
    *out_rate = 10;
    return (Interpolation_GetRate() - 0.5) * 10.0;

    // OG
fallback:
    *out_frame1 = &obj->frame_base[inv_item->current_frame];
    *out_frame2 = *out_frame1;
    *out_rate = 1;
    return 0;
}

static void M_DrawItem(
    const RING_INFO *const ring, const IMOTION_INFO *const motion,
    INVENTORY_ITEM *const inv_item, const int32_t num_frames)
{
    if (motion->status == RNG_DONE) {
        Output_SetLightAdder(LOW_LIGHT);
    } else if (inv_item == ring->list[ring->current_object]) {
        if (ring->rotating) {
            Output_SetLightAdder(LOW_LIGHT);
            for (int j = 0; j < num_frames; j++) {
                if (inv_item->y_rot < 0) {
                    inv_item->y_rot += 512;
                } else if (inv_item->y_rot > 0) {
                    inv_item->y_rot -= 512;
                }
            }
        } else if (
            motion->status == RNG_SELECTED || motion->status == RNG_DESELECTING
            || motion->status == RNG_SELECTING || motion->status == RNG_DESELECT
            || motion->status == RNG_CLOSING_ITEM) {
            Output_SetLightAdder(HIGH_LIGHT);
            for (int j = 0; j < num_frames; j++) {
                if (inv_item->y_rot != inv_item->y_rot_sel) {
                    if (inv_item->y_rot_sel - inv_item->y_rot > 0
                        && inv_item->y_rot_sel - inv_item->y_rot < 0x8000) {
                        inv_item->y_rot += 1024;
                    } else {
                        inv_item->y_rot -= 1024;
                    }
                    inv_item->y_rot &= 0xFC00u;
                }
            }
        } else if (
            ring->number_of_objects == 1
            || (!g_Input.menu_left && !g_Input.menu_right)
            || !g_Input.menu_left) {
            Output_SetLightAdder(HIGH_LIGHT);
            for (int j = 0; j < num_frames; j++) {
                inv_item->y_rot += 256;
            }
        }
    } else {
        Output_SetLightAdder(LOW_LIGHT);
        for (int j = 0; j < num_frames; j++) {
            if (inv_item->y_rot < 0) {
                inv_item->y_rot += 256;
            } else if (inv_item->y_rot > 0) {
                inv_item->y_rot -= 256;
            }
        }
    }

    Matrix_TranslateRel(0, inv_item->ytrans, inv_item->ztrans);
    Matrix_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);

    OBJECT *const obj = Object_GetObject(inv_item->object_id);
    if (obj->nmeshes < 0) {
        Output_DrawSpriteRel(0, 0, 0, obj->mesh_idx, 4096);
        return;
    }

    if (inv_item->sprlist) {
        int32_t zv = g_MatrixPtr->_23;
        int32_t zp = zv / g_PhdPersp;
        int32_t sx = Viewport_GetCenterX() + g_MatrixPtr->_03 / zp;
        int32_t sy = Viewport_GetCenterY() + g_MatrixPtr->_13 / zp;

        INVENTORY_SPRITE **sprlist = inv_item->sprlist;
        INVENTORY_SPRITE *spr;
        while ((spr = *sprlist++)) {
            if (zv < Output_GetNearZ() || zv > Output_GetFarZ()) {
                break;
            }

            while (spr->shape) {
                switch (spr->shape) {
                case SHAPE_SPRITE:
                    Output_DrawScreenSprite(
                        sx + spr->x, sy + spr->y, spr->z, spr->param1,
                        spr->param2,
                        Object_GetObject(O_ALPHABET)->mesh_idx + spr->sprnum,
                        4096, 0);
                    break;
                case SHAPE_LINE:
                    Output_DrawScreenLine(
                        sx + spr->x, sy + spr->y, spr->param1, spr->param2,
                        Output_RGB2RGBA(
                            Output_GetPaletteColor((uint8_t)spr->sprnum)));
                    break;
                case SHAPE_BOX: {
                    double scale = Viewport_GetHeight() / 480.0;
                    Output_DrawScreenBox(
                        sx + spr->x - scale, sy + spr->y - scale, spr->param1,
                        spr->param2, Text_GetMenuColor(MC_GOLD_DARK),
                        Text_GetMenuColor(MC_GOLD_LIGHT),
                        TEXT_OUTLINE_THICKNESS * scale);
                } break;
                case SHAPE_FBOX:
                    Output_DrawScreenFBox(
                        sx + spr->x, sy + spr->y, spr->param1, spr->param2);
                    break;
                }
                spr++;
            }
        }
    }

    int32_t rate;
    ANIM_FRAME *frame1;
    ANIM_FRAME *frame2;
    const int32_t frac =
        M_GetFrames(ring, motion, inv_item, &frame1, &frame2, &rate);
    if (inv_item->object_id == O_MAP_OPTION) {
        const int16_t extra_rotation[1] = { Option_Compass_GetNeedleAngle() };
        Object_GetBone(obj, 0)->rot_y = true;
        Object_DrawInterpolatedObject(
            obj, inv_item->drawn_meshes, extra_rotation, frame1, frame2, frac,
            rate);
    } else {
        Object_DrawInterpolatedObject(
            obj, inv_item->drawn_meshes, NULL, frame1, frame2, frac, rate);
    }
}

void InvRing_Draw(RING_INFO *const ring, IMOTION_INFO *const motion)
{
    const int32_t num_frames = Clock_GetFrameAdvance()
        * round(Clock_GetElapsedDrawFrames(&g_InvRing_MotionTimer));
    ring->camera.pos.z = ring->radius + CAMERA_2_RING;

    if (g_InvMode == INV_TITLE_MODE) {
        Output_DrawBackdropScreen();
        Interpolation_Commit();
    } else {
        Matrix_LookAt(
            g_InvRing_OldCamera.pos.x,
            g_InvRing_OldCamera.pos.y + g_InvRing_OldCamera.shift,
            g_InvRing_OldCamera.pos.z, g_InvRing_OldCamera.target.x,
            g_InvRing_OldCamera.target.y, g_InvRing_OldCamera.target.z, 0);
        Interpolation_Disable();
        Game_DrawScene(false);
        Interpolation_Enable();

        int32_t width = Screen_GetResWidth();
        int32_t height = Screen_GetResHeight();
        Viewport_Init(0, 0, width, height);
    }

    int16_t old_fov = Viewport_GetFOV();
    Viewport_SetFOV(PASSPORT_FOV * PHD_DEGREE);
    Output_ApplyFOV();

    Output_SetupAboveWater(false);

    XYZ_32 view_pos;
    XYZ_16 view_rot;
    InvRing_GetView(ring, &view_pos, &view_rot);
    Matrix_GenerateW2V(&view_pos, &view_rot);
    InvRing_Light(ring);

    Matrix_Push();
    Matrix_TranslateAbs(
        ring->ringpos.pos.x, ring->ringpos.pos.y, ring->ringpos.pos.z);
    Matrix_RotYXZ(
        ring->ringpos.rot.y, ring->ringpos.rot.x, ring->ringpos.rot.z);

    if (!(g_InvMode == INV_TITLE_MODE && Output_FadeIsAnimating()
          && motion->status == RNG_OPENING)) {
        PHD_ANGLE angle = 0;
        for (int i = 0; i < ring->number_of_objects; i++) {
            INVENTORY_ITEM *inv_item = ring->list[i];
            Matrix_Push();
            Matrix_RotYXZ(angle, 0, 0);
            Matrix_TranslateRel(ring->radius, 0, 0);
            Matrix_RotYXZ(PHD_90, inv_item->pt_xrot, 0);
            M_DrawItem(ring, motion, inv_item, num_frames);
            angle += ring->angle_adder;
            Matrix_Pop();
        }
    }

    INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
    switch (inv_item->object_id) {
    case O_MEDI_OPTION:
    case O_BIGMEDI_OPTION:
        if (g_Config.ui.enable_game_ui) {
            Overlay_BarDrawHealth();
        }
        break;

    default:
        break;
    }

    Matrix_Pop();
    Viewport_SetFOV(old_fov);

    Output_ClearDepthBuffer();
    if (motion->status == RNG_SELECTED) {
        INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
        if (inv_item->object_id == O_PASSPORT_CLOSED) {
            inv_item->object_id = O_PASSPORT_OPTION;
        }
        Option_Draw(inv_item);
    }

    if ((motion->status != RNG_OPENING
         || (g_InvMode != INV_TITLE_MODE || !Output_FadeIsAnimating()))
        && motion->status != RNG_DONE) {
        for (int i = 0; i < num_frames; i++) {
            InvRing_DoMotions(ring);
        }
    }
}
