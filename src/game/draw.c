#include "game/draw.h"

#include "config.h"
#include "game/inv.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "game/viewport.h"
#include "global/const.h"
#include "global/vars.h"
#include "math/matrix.h"
#include "util.h"

static int16_t m_InterpolatedBounds[6] = { 0 };

void DrawEffect(int16_t fxnum)
{
    FX_INFO *fx = &g_Effects[fxnum];
    OBJECT_INFO *object = &g_Objects[fx->object_number];
    if (!object->loaded) {
        return;
    }

    if (object->nmeshes < 0) {
        Output_DrawSprite(
            fx->pos.x, fx->pos.y, fx->pos.z,
            object->mesh_index - fx->frame_number, 4096);
    } else {
        Matrix_Push();
        Matrix_TranslateAbs(fx->pos.x, fx->pos.y, fx->pos.z);
        if (g_MatrixPtr->_23 > Output_GetNearZ()
            && g_MatrixPtr->_23 < Output_GetFarZ()) {
            Matrix_RotYXZ(fx->pos.y_rot, fx->pos.x_rot, fx->pos.z_rot);
            if (object->nmeshes) {
                Output_CalculateStaticLight(fx->shade);
                Output_DrawPolygons(g_Meshes[object->mesh_index], -1);
            } else {
                Output_CalculateLight(
                    fx->pos.x, fx->pos.y, fx->pos.z, fx->room_number);
                Output_DrawPolygons(g_Meshes[fx->frame_number], -1);
            }
        }
        Matrix_Pop();
    }
}

void DrawSpriteItem(ITEM_INFO *item)
{
    Output_DrawSprite(
        item->pos.x, item->pos.y, item->pos.z,
        g_Objects[item->object_number].mesh_index - item->frame_number,
        item->shade);
}

void DrawDummyItem(ITEM_INFO *item)
{
}

void DrawPickupItem(ITEM_INFO *item)
{
    if (!g_Config.enable_3d_pickups) {
        DrawSpriteItem(item);
        return;
    }

    // Convert item to menu display item.
    int16_t item_num_option = Inv_GetItemOption(item->object_number);
    // Save the frame number.
    int16_t old_frame_number = item->frame_number;
    // Modify item to be the anim for inv item and animation 0.
    item->anim_number = g_Objects[item_num_option].anim_index;
    item->frame_number = g_Anims[item->anim_number].frame_base;

    OBJECT_INFO *object = &g_Objects[item_num_option];

    int16_t *frmptr[2];
    int32_t rate;
    int32_t frac = GetFrames(item, frmptr, &rate);

    // Restore the old frame number in case we need to get the sprite again.
    item->frame_number = old_frame_number;

    // Fall back to normal sprite rendering if not found.
    if (object->nmeshes < 0) {
        DrawSpriteItem(item);
        return;
    }

    // Good news is there is a mesh, we just need to work out where to put it

    // First - Is there floor under the item?
    // This is mostly true, but for example the 4 items in the Obelisk of
    // Khamoon the 4 items are sitting on top of a static mesh which is not
    // floor.
    FLOOR_INFO *floor = Room_GetFloor(
        item->pos.x, item->pos.y, item->pos.z, &item->room_number);
    int16_t floor_height =
        Room_GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);

    // Assume this is our offset.
    int16_t offset = floor_height;
    // Is the floor "just below" the item?
    int16_t floor_mapped_delta = ABS(floor_height - item->pos.y);
    if (floor_mapped_delta > WALL_L / 4 || floor_mapped_delta == 0) {
        // No, now we need to move it a bit.
        // First get the sprite that was to be used,

        int16_t spr_num =
            g_Objects[item->object_number].mesh_index - item->frame_number;
        PHD_SPRITE *sprite = &g_PhdSpriteInfo[spr_num];

        // and get the animation bounding box, which is not the mesh one.
        int16_t min_y = frmptr[0][FRAME_BOUND_MIN_Y];
        int16_t max_y = frmptr[0][FRAME_BOUND_MAX_Y];
        int16_t anim_y = frmptr[0][FRAME_POS_Y];

        // Different objects need different heuristics.
        switch (item_num_option) {
        case O_GUN_OPTION:
        case O_SHOTGUN_OPTION:
        case O_MAGNUM_OPTION:
        case O_UZI_OPTION:
        case O_MAG_AMMO_OPTION:
        case O_UZI_AMMO_OPTION:
        case O_EXPLOSIVE_OPTION:
        case O_LEADBAR_OPTION:
        case O_PICKUP_OPTION1:
        case O_PICKUP_OPTION2:
        case O_SCION_OPTION:
            // Ignore the sprite and just position based upon the anim.
            offset = item->pos.y + (min_y - anim_y) / 2;
            break;
        case O_MEDI_OPTION:
        case O_BIGMEDI_OPTION:
        case O_SG_AMMO_OPTION:
        case O_PUZZLE_OPTION1:
        case O_PUZZLE_OPTION2:
        case O_PUZZLE_OPTION3:
        case O_PUZZLE_OPTION4:
        case O_KEY_OPTION1:
        case O_KEY_OPTION2:
        case O_KEY_OPTION3:
        case O_KEY_OPTION4: {
            // Take the difference from the bottom of the sprite and the bottom
            // of the animation and divide it by 8.
            // 8 was chosen because in testing it positioned objects correctly.
            // Specifically the 4 items in the Obelisk of Khamoon and keys.
            // Some objects have a centred mesh and some have one that is from
            // the bottom, for the centred ones; move up from the
            // bottom is necessary.
            int centred = ABS(min_y + max_y) < 8;
            if (floor_mapped_delta) {
                offset = item->pos.y - ABS(min_y - sprite->y1) / 8;
            } else if (centred) {
                offset = item->pos.y + min_y;
            }
            break;
        }
        }
    }

    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, offset, item->pos.z);

    Output_CalculateLight(
        item->pos.x, item->pos.y, item->pos.z, item->room_number);

    int16_t *frame = &object->frame_base[object->nmeshes * 2 + 10];
    int32_t clip = Output_GetObjectBounds(frame);
    if (clip) {
        // From this point on the function is a slightly customised version
        // of the code in DrawAnimatingItem starting with the line that
        // matches the following line.
        int32_t bit = 1;
        int16_t **meshpp = &g_Meshes[object->mesh_index];
        int32_t *bone = &g_AnimBones[object->bone_index];

        if (!frac) {
            Matrix_TranslateRel(
                frmptr[0][FRAME_POS_X], frmptr[0][FRAME_POS_Y],
                frmptr[0][FRAME_POS_Z]);

            int32_t *packed_rotation = (int32_t *)(frmptr[0] + FRAME_ROT);
            Matrix_RotYXZpack(*packed_rotation++);

            if (item->mesh_bits & bit) {
                Output_DrawPolygons(*meshpp++, clip);
            }

            for (int i = 1; i < object->nmeshes; i++) {
                int32_t bone_extra_flags = *bone;
                if (bone_extra_flags & BEB_POP) {
                    Matrix_Pop();
                }

                if (bone_extra_flags & BEB_PUSH) {
                    Matrix_Push();
                }

                Matrix_TranslateRel(bone[1], bone[2], bone[3]);
                Matrix_RotYXZpack(*packed_rotation++);

                // Extra rotation is ignored in this case as it's not needed.

                bit <<= 1;
                if (item->mesh_bits & bit) {
                    Output_DrawPolygons(*meshpp, clip);
                }

                bone += 4;
                meshpp++;
            }
        } else {
            // This should never happen but is here "just in case".
            Matrix_InitInterpolate(frac, rate);
            Matrix_TranslateRel_ID(
                frmptr[0][FRAME_POS_X], frmptr[0][FRAME_POS_Y],
                frmptr[0][FRAME_POS_Z], frmptr[1][FRAME_POS_X],
                frmptr[1][FRAME_POS_Y], frmptr[1][FRAME_POS_Z]);
            int32_t *packed_rotation1 = (int32_t *)(frmptr[0] + FRAME_ROT);
            int32_t *packed_rotation2 = (int32_t *)(frmptr[1] + FRAME_ROT);
            Matrix_RotYXZpack_I(*packed_rotation1++, *packed_rotation2++);

            if (item->mesh_bits & bit) {
                Output_DrawPolygons_I(*meshpp++, clip);
            }

            for (int i = 1; i < object->nmeshes; i++) {
                int32_t bone_extra_flags = *bone;
                if (bone_extra_flags & BEB_POP) {
                    Matrix_Pop_I();
                }

                if (bone_extra_flags & BEB_PUSH) {
                    Matrix_Push_I();
                }

                Matrix_TranslateRel_I(bone[1], bone[2], bone[3]);
                Matrix_RotYXZpack_I(*packed_rotation1++, *packed_rotation2++);

                // Extra rotation is ignored in this case as it's not needed.

                bit <<= 1;
                if (item->mesh_bits & bit) {
                    Output_DrawPolygons_I(*meshpp, clip);
                }

                bone += 4;
                meshpp++;
            }
        }
    }

    Matrix_Pop();
}

void DrawAnimatingItem(ITEM_INFO *item)
{
    static int16_t null_rotation[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    int16_t *frmptr[2];
    int32_t rate;
    int32_t frac = GetFrames(item, frmptr, &rate);
    OBJECT_INFO *object = &g_Objects[item->object_number];

    if (object->shadow_size) {
        Output_DrawShadow(object->shadow_size, frmptr[0], item);
    }

    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
    Matrix_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    int32_t clip = Output_GetObjectBounds(frmptr[0]);
    if (!clip) {
        Matrix_Pop();
        return;
    }

    CalculateObjectLighting(item, frmptr[0]);
    int16_t *extra_rotation = item->data ? item->data : &null_rotation;

    int32_t bit = 1;
    int16_t **meshpp = &g_Meshes[object->mesh_index];
    int32_t *bone = &g_AnimBones[object->bone_index];

    if (!frac) {
        Matrix_TranslateRel(
            frmptr[0][FRAME_POS_X], frmptr[0][FRAME_POS_Y],
            frmptr[0][FRAME_POS_Z]);

        int32_t *packed_rotation = (int32_t *)(frmptr[0] + FRAME_ROT);
        Matrix_RotYXZpack(*packed_rotation++);

        if (item->mesh_bits & bit) {
            Output_DrawPolygons(*meshpp++, clip);
        }

        for (int i = 1; i < object->nmeshes; i++) {
            int32_t bone_extra_flags = *bone;
            if (bone_extra_flags & BEB_POP) {
                Matrix_Pop();
            }

            if (bone_extra_flags & BEB_PUSH) {
                Matrix_Push();
            }

            Matrix_TranslateRel(bone[1], bone[2], bone[3]);
            Matrix_RotYXZpack(*packed_rotation++);

            if (bone_extra_flags & BEB_ROT_Y) {
                Matrix_RotY(*extra_rotation++);
            }
            if (bone_extra_flags & BEB_ROT_X) {
                Matrix_RotX(*extra_rotation++);
            }
            if (bone_extra_flags & BEB_ROT_Z) {
                Matrix_RotZ(*extra_rotation++);
            }

            bit <<= 1;
            if (item->mesh_bits & bit) {
                Output_DrawPolygons(*meshpp, clip);
            }

            bone += 4;
            meshpp++;
        }
    } else {
        Matrix_InitInterpolate(frac, rate);
        Matrix_TranslateRel_ID(
            frmptr[0][FRAME_POS_X], frmptr[0][FRAME_POS_Y],
            frmptr[0][FRAME_POS_Z], frmptr[1][FRAME_POS_X],
            frmptr[1][FRAME_POS_Y], frmptr[1][FRAME_POS_Z]);
        int32_t *packed_rotation1 = (int32_t *)(frmptr[0] + FRAME_ROT);
        int32_t *packed_rotation2 = (int32_t *)(frmptr[1] + FRAME_ROT);
        Matrix_RotYXZpack_I(*packed_rotation1++, *packed_rotation2++);

        if (item->mesh_bits & bit) {
            Output_DrawPolygons_I(*meshpp++, clip);
        }

        for (int i = 1; i < object->nmeshes; i++) {
            int32_t bone_extra_flags = *bone;
            if (bone_extra_flags & BEB_POP) {
                Matrix_Pop_I();
            }

            if (bone_extra_flags & BEB_PUSH) {
                Matrix_Push_I();
            }

            Matrix_TranslateRel_I(bone[1], bone[2], bone[3]);
            Matrix_RotYXZpack_I(*packed_rotation1++, *packed_rotation2++);

            if (bone_extra_flags & BEB_ROT_Y) {
                Matrix_RotY_I(*extra_rotation++);
            }
            if (bone_extra_flags & BEB_ROT_X) {
                Matrix_RotX_I(*extra_rotation++);
            }
            if (bone_extra_flags & BEB_ROT_Z) {
                Matrix_RotZ_I(*extra_rotation++);
            }

            bit <<= 1;
            if (item->mesh_bits & bit) {
                Output_DrawPolygons_I(*meshpp, clip);
            }

            bone += 4;
            meshpp++;
        }
    }

    Matrix_Pop();
}

void DrawUnclippedItem(ITEM_INFO *item)
{
    int32_t left = g_PhdLeft;
    int32_t top = g_PhdTop;
    int32_t right = g_PhdRight;
    int32_t bottom = g_PhdBottom;

    g_PhdLeft = Viewport_GetMinX();
    g_PhdTop = Viewport_GetMinY();
    g_PhdRight = Viewport_GetMaxX();
    g_PhdBottom = Viewport_GetMaxY();

    DrawAnimatingItem(item);

    g_PhdLeft = left;
    g_PhdTop = top;
    g_PhdRight = right;
    g_PhdBottom = bottom;
}

void DrawGunFlash(int32_t weapon_type, int32_t clip)
{
    int32_t light;
    int32_t len;
    int32_t off;

    switch (weapon_type) {
    case LGT_MAGNUMS:
        light = 16 * 256;
        len = 155;
        off = 55;
        break;

    case LGT_UZIS:
        light = 10 * 256;
        len = 180;
        off = 55;
        break;

    case LGT_SHOTGUN:
        light = 10 * 256;
        len = 285;
        off = 0;
        break;

    default:
        light = 20 * 256;
        len = 155;
        off = 55;
        break;
    }

    Matrix_TranslateRel(0, len, off);
    Matrix_RotYXZ(0, -90 * PHD_DEGREE, (PHD_ANGLE)(Random_GetDraw() * 2));
    Output_CalculateStaticLight(light);
    Output_DrawPolygons(g_Meshes[g_Objects[O_GUN_FLASH].mesh_index], clip);
}

void CalculateObjectLighting(ITEM_INFO *item, int16_t *frame)
{
    if (item->shade >= 0) {
        Output_CalculateStaticLight(item->shade);
        return;
    }

    Matrix_PushUnit();
    g_MatrixPtr->_23 = 0;
    g_MatrixPtr->_13 = 0;
    g_MatrixPtr->_03 = 0;

    Matrix_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    Matrix_TranslateRel(
        (frame[FRAME_BOUND_MIN_X] + frame[FRAME_BOUND_MAX_X]) / 2,
        (frame[FRAME_BOUND_MIN_Y] + frame[FRAME_BOUND_MAX_Y]) / 2,
        (frame[FRAME_BOUND_MIN_Z] + frame[FRAME_BOUND_MAX_Z]) / 2);

    int32_t x = (g_MatrixPtr->_03 >> W2V_SHIFT) + item->pos.x;
    int32_t y = (g_MatrixPtr->_13 >> W2V_SHIFT) + item->pos.y;
    int32_t z = (g_MatrixPtr->_23 >> W2V_SHIFT) + item->pos.z;

    Matrix_Pop();

    Output_CalculateLight(x, y, z, item->room_number);
}

int32_t GetFrames(ITEM_INFO *item, int16_t *frmptr[], int32_t *rate)
{
    ANIM_STRUCT *anim = &g_Anims[item->anim_number];
    frmptr[0] = anim->frame_ptr;
    frmptr[1] = anim->frame_ptr;

    *rate = anim->interpolation;

    int32_t frm = item->frame_number - anim->frame_base;
    int32_t first = frm / anim->interpolation;
    int32_t frame_size = g_Objects[item->object_number].nmeshes * 2 + 10;

    frmptr[0] += first * frame_size;
    frmptr[1] = frmptr[0] + frame_size;

    int32_t interp = frm % anim->interpolation;
    if (!interp) {
        return 0;
    }

    int32_t second = anim->interpolation * (first + 1);
    if (second > anim->frame_end) {
        *rate = anim->frame_end + anim->interpolation - second;
    }

    return interp;
}

int16_t *GetBoundsAccurate(ITEM_INFO *item)
{
    int32_t rate;
    int16_t *frmptr[2];

    int32_t frac = GetFrames(item, frmptr, &rate);
    if (!frac) {
        return frmptr[0];
    }

    for (int i = 0; i < 6; i++) {
        int16_t a = frmptr[0][i];
        int16_t b = frmptr[1][i];
        m_InterpolatedBounds[i] = a + (((b - a) * frac) / rate);
    }
    return m_InterpolatedBounds;
}
