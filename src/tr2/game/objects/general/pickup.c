#include "game/objects/general/pickup.h"

#include "config.h"
#include "decomp/flares.h"
#include "game/gameflow.h"
#include "game/gun/gun.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/inventory_ring.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/lara/misc.h"
#include "game/matrix.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/room.h"
#include "global/vars.h"

#define LF_PICKUP_ERASE 42
#define LF_PICKUP_FLARE 58
#define LF_PICKUP_FLARE_UW 20
#define LF_PICKUP_UW 18

int16_t g_PickupBounds[12] = {
    -WALL_L / 4,      +WALL_L / 4,      -100, +100, -WALL_L / 4, +WALL_L / 4,
    -10 * PHD_DEGREE, +10 * PHD_DEGREE, +0,   +0,   +0,          +0,
};

static XYZ_32 m_PickupPosition = { .x = 0, .y = 0, .z = -100 };
static XYZ_32 m_PickupPositionUW = { .x = 0, .y = -200, .z = -350 };

static int16_t m_PickupBoundsUW[12] = {
    -WALL_L / 2,      +WALL_L / 2,      -WALL_L / 2,      +WALL_L / 2,
    -WALL_L / 2,      +WALL_L / 2,      -45 * PHD_DEGREE, +45 * PHD_DEGREE,
    -45 * PHD_DEGREE, +45 * PHD_DEGREE, -45 * PHD_DEGREE, +45 * PHD_DEGREE,
};

static void M_DoPickup(int16_t item_num);
static void M_DoFlarePickup(int16_t item_num);

static void M_DoAboveWater(int16_t item, ITEM *lara_item);
static void M_DoUnderwater(int16_t item, ITEM *lara_item);

static void M_DoPickup(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (item->object_id == O_FLARE_ITEM) {
        return;
    }

    Overlay_AddDisplayPickup(item->object_id);
    Inv_AddItem(item->object_id);

    if ((item->object_id == O_SECRET_1 || item->object_id == O_SECRET_2
         || item->object_id == O_SECRET_3)
        && (g_SaveGame.current_stats.secrets_bitmap & 1)
                + ((g_SaveGame.current_stats.secrets_bitmap >> 1) & 1)
                + ((g_SaveGame.current_stats.secrets_bitmap >> 2) & 1)
            >= 3) {
        GF_ModifyInventory(g_CurrentLevel, 1);
    }

    item->status = IS_INVISIBLE;
    Item_RemoveDrawn(item_num);
}

static void M_DoFlarePickup(const int16_t item_num)
{
    const ITEM *const item = Item_Get(item_num);
    g_Lara.request_gun_type = LGT_FLARE;
    g_Lara.gun_type = LGT_FLARE;
    Gun_InitialiseNewWeapon();
    g_Lara.gun_status = LGS_SPECIAL;
    g_Lara.flare_age = (int32_t)item->data & 0x7FFF;
    Item_Kill(item_num);
}

static void M_DoAboveWater(const int16_t item_num, ITEM *const lara_item)
{
    ITEM *const item = Item_Get(item_num);
    const XYZ_16 old_rot = item->rot;

    item->rot.x = 0;
    item->rot.y = lara_item->rot.y;
    item->rot.z = 0;

    if (!Item_TestPosition(g_PickupBounds, item, lara_item)) {
        goto cleanup;
    }

    if (lara_item->current_anim_state == LS_PICKUP) {
        if (lara_item->frame_num
            == g_Anims[LA_PICKUP].frame_base + LF_PICKUP_ERASE) {
            M_DoPickup(item_num);
        }
        goto cleanup;
    }

    if (lara_item->current_anim_state == LS_FLARE_PICKUP) {
        if (lara_item->frame_num
                == g_Anims[LA_FLARE_PICKUP].frame_base + LF_PICKUP_FLARE
            && item->object_id == O_FLARE_ITEM
            && g_Lara.gun_type != LGT_FLARE) {
            M_DoFlarePickup(item_num);
        }
        goto cleanup;
    }

    if (g_Input.action && !lara_item->gravity
        && lara_item->current_anim_state == LS_STOP
        && g_Lara.gun_status == LGS_ARMLESS
        && (g_Lara.gun_type != LGT_FLARE || item->object_id != O_FLARE_ITEM)) {
        if (item->object_id == O_FLARE_ITEM) {
            lara_item->goal_anim_state = LS_FLARE_PICKUP;
            do {
                Lara_Animate(lara_item);
            } while (lara_item->current_anim_state != LS_FLARE_PICKUP);
            lara_item->goal_anim_state = LS_STOP;
            g_Lara.gun_status = LGS_HANDS_BUSY;
        } else {
            Item_AlignPosition(&m_PickupPosition, item, lara_item);
            lara_item->goal_anim_state = LS_PICKUP;
            do {
                Lara_Animate(lara_item);
            } while (lara_item->current_anim_state != LS_PICKUP);
            lara_item->goal_anim_state = LS_STOP;
            g_Lara.gun_status = LGS_HANDS_BUSY;
        }
        goto cleanup;
    }

cleanup:
    item->rot = old_rot;
}

static void M_DoUnderwater(const int16_t item_num, ITEM *const lara_item)
{
    ITEM *const item = Item_Get(item_num);
    const XYZ_16 old_rot = item->rot;

    item->rot.x = -25 * PHD_DEGREE;
    item->rot.y = lara_item->rot.y;
    item->rot.z = 0;

    if (!Item_TestPosition(m_PickupBoundsUW, item, lara_item)) {
        goto cleanup;
    }

    if (lara_item->current_anim_state == LS_PICKUP) {
        if (lara_item->frame_num
            == g_Anims[LA_UNDERWATER_PICKUP].frame_base + LF_PICKUP_UW) {
            M_DoPickup(item_num);
        }
        goto cleanup;
    }

    if (lara_item->current_anim_state == LS_FLARE_PICKUP) {
        if (lara_item->frame_num
                == g_Anims[LA_UNDERWATER_FLARE_PICKUP].frame_base
                    + LF_PICKUP_FLARE_UW
            && item->object_id == O_FLARE_ITEM
            && g_Lara.gun_type != LGT_FLARE) {
            M_DoFlarePickup(item_num);
            Flare_DrawMeshes();
        }
        goto cleanup;
    }

    if (g_Input.action && lara_item->current_anim_state == LS_TREAD
        && g_Lara.gun_status == LGS_ARMLESS
        && (g_Lara.gun_type != LGT_FLARE || item->object_id != O_FLARE_ITEM)) {
        if (!Lara_MovePosition(&m_PickupPositionUW, item, lara_item)) {
            goto cleanup;
        }

        if (item->object_id == O_FLARE_ITEM) {
            lara_item->fall_speed = 0;
            lara_item->anim_num = LA_UNDERWATER_FLARE_PICKUP;
            lara_item->frame_num = g_Anims[lara_item->anim_num].frame_base;
            lara_item->goal_anim_state = LS_TREAD;
            lara_item->current_anim_state = LS_FLARE_PICKUP;
        } else {
            if (g_Config.gameplay.fix_pickup_drift_glitch) {
                lara_item->fall_speed = 0;
            }
            lara_item->goal_anim_state = LS_PICKUP;
            do {
                Lara_Animate(lara_item);
            } while (lara_item->current_anim_state != LS_PICKUP);
            lara_item->goal_anim_state = LS_TREAD;
        }
        goto cleanup;
    }

cleanup:
    item->rot = old_rot;
}

void Pickup_Setup(OBJECT *const obj)
{
    obj->collision = Pickup_Collision;
    obj->draw_routine = Pickup_Draw;
    obj->save_position = 1;
    obj->save_flags = 1;
}

void Pickup_Draw(const ITEM *const item)
{
    if (!g_Config.visuals.enable_3d_pickups) {
        Object_DrawSpriteItem(item);
        return;
    }

    if (!g_Objects[item->object_id].loaded) {
        Object_DrawSpriteItem(item);
        return;
    }

    // Convert item to menu display item.
    const GAME_OBJECT_ID inv_object_id = Inv_GetItemOption(item->object_id);
    const OBJECT *const obj = Object_GetObject(inv_object_id);
    if (!obj->loaded || obj->mesh_count < 0) {
        Object_DrawSpriteItem(item);
        return;
    }

    // Get the first frame of the first animation, and its bounding box.
    int16_t offset;
    BOUNDS_16 bounds;
    const FRAME_INFO *frame = NULL;

    // Some items, such as the Prayer Wheel in Barkhang Monastery, do not have
    // animations, and for such items we need to calculate this information
    // manually.
    if (obj->anim_idx != -1) {
        frame = (const FRAME_INFO *)obj->frame_base;
        bounds = frame->bounds;
        const int16_t y_off = frame->offset.y - bounds.max_y;
        bounds.max_y -= bounds.max_y;
        bounds.min_y -= bounds.max_y;
        offset = item->pos.y + y_off;
    } else {
        bounds = Object_GetBoundingBox(obj, NULL, item->mesh_bits);
        offset = item->pos.y - (bounds.max_y - bounds.min_y) / 2;
    }

    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, offset, item->pos.z);
    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);

    Output_CalculateLight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);

    const int32_t clip = Output_GetObjectBounds(&bounds);
    if (clip) {
        int32_t bit = 1;
        int16_t **meshpp = &g_Meshes[obj->mesh_idx];
        int32_t *bone = &g_AnimBones[obj->bone_idx];

        const int16_t *mesh_rots = frame != NULL ? frame->mesh_rots : NULL;
        if (mesh_rots != NULL) {
            Matrix_RotYXZsuperpack(&mesh_rots, 0);
        }

        if (item->mesh_bits & bit) {
            Output_InsertPolygons(*meshpp++, clip);
        }

        for (int i = 1; i < obj->mesh_count; i++) {
            int32_t bone_extra_flags = *bone;
            if (bone_extra_flags & BF_MATRIX_POP) {
                Matrix_Pop();
            }

            if (bone_extra_flags & BF_MATRIX_PUSH) {
                Matrix_Push();
            }

            Matrix_TranslateRel(bone[1], bone[2], bone[3]);
            if (mesh_rots != NULL) {
                Matrix_RotYXZsuperpack(&mesh_rots, 0);
            }

            // Extra rotation is ignored in this case as it's not needed.

            bit <<= 1;
            if (item->mesh_bits & bit) {
                Output_InsertPolygons(*meshpp, clip);
            }

            bone += 4;
            meshpp++;
        }
    }

    Matrix_Pop();
}

void Pickup_Collision(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    if (g_Lara.water_status == LWS_ABOVE_WATER
        || g_Lara.water_status == LWS_WADE) {
        M_DoAboveWater(item_num, lara_item);
    } else if (g_Lara.water_status == LWS_UNDERWATER) {
        M_DoUnderwater(item_num, lara_item);
    }
}

int32_t Pickup_Trigger(int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (item->status != IS_INVISIBLE) {
        return false;
    }

    item->status = IS_DEACTIVATED;
    return true;
}
