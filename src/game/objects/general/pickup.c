#include "game/objects/general/pickup.h"

#include "config.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/objects/common.h"
#include "game/overlay.h"
#include "global/const.h"
#include "global/vars.h"

#define LF_PICKUP_ERASE 42
#define LF_PICKUP_UW 18

static XYZ_32 m_PickUpPosition = { 0, 0, -100 };
static XYZ_32 m_PickUpPositionUW = { 0, -200, -350 };

static OBJECT_BOUNDS m_PickUpBounds = {
    .min_shift_x = -256,
    .max_shift_x = +256,
    .min_shift_y = -100,
    .max_shift_y = +100,
    .min_shift_z = -256,
    .max_shift_z = +256,
    .min_rot_x = -10 * PHD_DEGREE,
    .max_rot_x = +10 * PHD_DEGREE,
    .min_rot_y = 0,
    .max_rot_y = 0,
    .min_rot_z = 0,
    .max_rot_z = 0,
};

static OBJECT_BOUNDS m_PickUpBoundsControlled = {
    .min_shift_x = -256,
    .max_shift_x = +256,
    .min_shift_y = -200,
    .max_shift_y = +200,
    .min_shift_z = -256,
    .max_shift_z = +256,
    .min_rot_x = -10 * PHD_DEGREE,
    .max_rot_x = +10 * PHD_DEGREE,
    .min_rot_y = 0,
    .max_rot_y = 0,
    .min_rot_z = 0,
    .max_rot_z = 0,
};

static OBJECT_BOUNDS m_PickUpBoundsUW = {
    .min_shift_x = -512,
    .max_shift_x = +512,
    .min_shift_y = -512,
    .max_shift_y = +512,
    .min_shift_z = -512,
    .max_shift_z = +512,
    .min_rot_x = -45 * PHD_DEGREE,
    .max_rot_x = +45 * PHD_DEGREE,
    .min_rot_y = -45 * PHD_DEGREE,
    .max_rot_y = +45 * PHD_DEGREE,
    .min_rot_z = -45 * PHD_DEGREE,
    .max_rot_z = +45 * PHD_DEGREE,
};

static void PickUp_GetItem(
    int16_t item_num, ITEM_INFO *item, ITEM_INFO *lara_item);
static void PickUp_GetAllAtLaraPos(ITEM_INFO *item, ITEM_INFO *lara_item);

static void PickUp_GetItem(
    int16_t item_num, ITEM_INFO *item, ITEM_INFO *lara_item)
{
    if (item->object_number == O_SHOTGUN_ITEM) {
        g_Lara.mesh_ptrs[LM_TORSO] =
            g_Meshes[g_Objects[O_SHOTGUN].mesh_index + LM_TORSO];
    }
    Overlay_AddPickup(item->object_number);
    Inv_AddItem(item->object_number);
    item->status = IS_INVISIBLE;
    Item_RemoveDrawn(item_num);
    g_GameInfo.current[g_CurrentLevel].stats.pickup_count++;
    g_Lara.interact_target.is_moving = false;
}

static void PickUp_GetAllAtLaraPos(ITEM_INFO *item, ITEM_INFO *lara_item)
{
    int16_t pickup_num = g_RoomInfo[item->room_number].item_number;
    while (pickup_num != NO_ITEM) {
        ITEM_INFO *check_item = &g_Items[pickup_num];
        if (check_item->pos.x == item->pos.x && check_item->pos.z == item->pos.z
            && g_Objects[check_item->object_number].collision
                == Pickup_Collision) {
            PickUp_GetItem(pickup_num, check_item, lara_item);
        }
        pickup_num = check_item->next_item;
    }
}

void Pickup_Setup(OBJECT_INFO *obj)
{
    obj->draw_routine = Object_DrawPickupItem;
    obj->collision = Pickup_Collision;
    obj->save_flags = 1;
}

void Pickup_Collision(int16_t item_num, ITEM_INFO *lara_item, COLL_INFO *coll)
{
    if (g_Config.walk_to_items) {
        Pickup_CollisionControlled(item_num, lara_item, coll);
        return;
    }

    ITEM_INFO *item = &g_Items[item_num];
    int16_t rotx = item->rot.x;
    int16_t roty = item->rot.y;
    int16_t rotz = item->rot.z;
    item->rot.y = lara_item->rot.y;
    item->rot.z = 0;

    if (g_Lara.water_status == LWS_ABOVE_WATER) {
        item->rot.x = 0;
        if (!Lara_TestPosition(item, &m_PickUpBounds)) {
            goto cleanup;
        }

        if (lara_item->current_anim_state == LS_PICKUP) {
            if (!Item_TestFrameEqual(lara_item, LF_PICKUP_ERASE)) {
                goto cleanup;
            }
            PickUp_GetAllAtLaraPos(item, lara_item);
            goto cleanup;
        }

        if (g_Input.action && g_Lara.gun_status == LGS_ARMLESS
            && !lara_item->gravity_status
            && lara_item->current_anim_state == LS_STOP) {
            Lara_AlignPosition(item, &m_PickUpPosition);
            Lara_AnimateUntil(lara_item, LS_PICKUP);
            lara_item->goal_anim_state = LS_STOP;
            g_Lara.gun_status = LGS_HANDS_BUSY;
            goto cleanup;
        }
    } else if (g_Lara.water_status == LWS_UNDERWATER) {
        item->rot.x = -25 * PHD_DEGREE;
        if (!Lara_TestPosition(item, &m_PickUpBoundsUW)) {
            goto cleanup;
        }

        if (lara_item->current_anim_state == LS_PICKUP) {
            if (!Item_TestFrameEqual(lara_item, LF_PICKUP_UW)) {
                goto cleanup;
            }
            PickUp_GetAllAtLaraPos(item, lara_item);
            goto cleanup;
        }

        if (g_Input.action && lara_item->current_anim_state == LS_TREAD) {
            if (!Lara_MovePosition(item, &m_PickUpPositionUW)) {
                goto cleanup;
            }
            Lara_AnimateUntil(lara_item, LS_PICKUP);
            lara_item->goal_anim_state = LS_TREAD;
        }
    }

cleanup:
    item->rot.x = rotx;
    item->rot.y = roty;
    item->rot.z = rotz;
}

void Pickup_CollisionControlled(
    int16_t item_num, ITEM_INFO *lara_item, COLL_INFO *coll)
{
    ITEM_INFO *item = &g_Items[item_num];

    if (item->status == IS_INVISIBLE) {
        return;
    }

    bool have_item = false;
    int16_t rotx = item->rot.x;
    int16_t roty = item->rot.y;
    int16_t rotz = item->rot.z;
    item->rot.y = lara_item->rot.y;
    item->rot.z = 0;

    if (g_Lara.water_status == LWS_ABOVE_WATER) {
        if ((g_Input.action && g_Lara.gun_status == LGS_ARMLESS
             && !lara_item->gravity_status
             && lara_item->current_anim_state == LS_STOP
             && !g_Lara.interact_target.is_moving)
            || (g_Lara.interact_target.is_moving
                && g_Lara.interact_target.item_num == item_num)) {

            have_item = false;
            item->rot.x = 0;

            if (Lara_TestPosition(item, &m_PickUpBoundsControlled)) {
                m_PickUpPosition.y = lara_item->pos.y - item->pos.y;
                if (Lara_MovePosition(item, &m_PickUpPosition)) {
                    Item_SwitchToAnim(lara_item, LA_PICKUP, 0);
                    lara_item->current_anim_state = LS_PICKUP;
                    have_item = true;
                }
                g_Lara.interact_target.item_num = item_num;
            } else if (
                g_Lara.interact_target.is_moving
                && g_Lara.interact_target.item_num == item_num) {
                g_Lara.interact_target.is_moving = false;
                g_Lara.gun_status = LGS_ARMLESS;
            }
            if (have_item) {
                g_Lara.head_rot.y = 0;
                g_Lara.head_rot.x = 0;
                g_Lara.torso_rot.y = 0;
                g_Lara.torso_rot.x = 0;
                g_Lara.interact_target.is_moving = false;
                g_Lara.gun_status = LGS_HANDS_BUSY;
            }
        } else if (
            g_Lara.interact_target.item_num == item_num
            && lara_item->current_anim_state == LS_PICKUP) {
            if (Item_TestFrameEqual(lara_item, LF_PICKUP_ERASE)) {
                PickUp_GetAllAtLaraPos(item, lara_item);
            }
        }
    } else if (g_Lara.water_status == LWS_UNDERWATER) {
        item->rot.x = -25 * PHD_DEGREE;

        if ((g_Input.action && lara_item->current_anim_state == LS_TREAD
             && g_Lara.gun_status == LGS_ARMLESS
             && !g_Lara.interact_target.is_moving)
            || (g_Lara.interact_target.is_moving
                && g_Lara.interact_target.item_num == item_num)) {

            if (Lara_TestPosition(item, &m_PickUpBoundsUW)) {
                if (Lara_MovePosition(item, &m_PickUpPositionUW)) {
                    Item_SwitchToAnim(lara_item, LA_PICKUP_UW, 0);
                    lara_item->current_anim_state = LS_PICKUP;

                    lara_item->goal_anim_state = LS_TREAD;
                    g_Lara.interact_target.is_moving = false;
                    g_Lara.gun_status = LGS_HANDS_BUSY;
                }
                g_Lara.interact_target.item_num = item_num;
            } else if (
                g_Lara.interact_target.is_moving
                && g_Lara.interact_target.item_num == item_num) {
                g_Lara.interact_target.is_moving = false;
                g_Lara.gun_status = LGS_ARMLESS;
            }
        } else if (
            g_Lara.interact_target.item_num == item_num
            && lara_item->current_anim_state == LS_PICKUP
            && Item_TestFrameEqual(lara_item, LF_PICKUP_UW)) {
            PickUp_GetAllAtLaraPos(item, lara_item);
            g_Lara.gun_status = LGS_ARMLESS;
        }
    }
    item->rot.x = rotx;
    item->rot.y = roty;
    item->rot.z = rotz;
}

bool Pickup_Trigger(int16_t item_num)
{
    ITEM_INFO *item = &g_Items[item_num];
    if (item->status != IS_INVISIBLE) {
        return false;
    }
    item->status = IS_DEACTIVATED;
    return true;
}
