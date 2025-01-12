#include "game/lara/control.h"

#include "decomp/skidoo.h"
#include "game/creature.h"
#include "game/gun/gun.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/item_actions.h"
#include "game/items.h"
#include "game/lara/cheat.h"
#include "game/lara/col.h"
#include "game/lara/look.h"
#include "game/lara/misc.h"
#include "game/lara/state.h"
#include "game/music.h"
#include "game/room.h"
#include "game/sound.h"
#include "game/spawn.h"
#include "global/const.h"
#include "global/vars.h"

#include <libtrx/game/math.h>
#include <libtrx/utils.h>

static int32_t m_OpenDoorsCheatCooldown = 0;

static void (*m_ControlRoutines[])(ITEM *item, COLL_INFO *coll) = {
    // clang-format off
    [LS_WALK]         = Lara_State_Walk,
    [LS_RUN]          = Lara_State_Run,
    [LS_STOP]         = Lara_State_Stop,
    [LS_FORWARD_JUMP] = Lara_State_ForwardJump,
    [LS_POSE]         = Lara_State_Empty,
    [LS_FAST_BACK]    = Lara_State_FastBack,
    [LS_TURN_RIGHT]   = Lara_State_TurnRight,
    [LS_TURN_LEFT]    = Lara_State_TurnLeft,
    [LS_DEATH]        = Lara_State_Death,
    [LS_FAST_FALL]    = Lara_State_FastFall,
    [LS_HANG]         = Lara_State_Hang,
    [LS_REACH]        = Lara_State_Reach,
    [LS_SPLAT]        = Lara_State_Splat,
    [LS_TREAD]        = Lara_State_Tread,
    [LS_LAND]         = Lara_State_Empty,
    [LS_COMPRESS]     = Lara_State_Compress,
    [LS_BACK]         = Lara_State_Back,
    [LS_SWIM]         = Lara_State_Swim,
    [LS_GLIDE]        = Lara_State_Glide,
    [LS_NULL]         = Lara_State_Null,
    [LS_FAST_TURN]    = Lara_State_FastTurn,
    [LS_STEP_RIGHT]   = Lara_State_StepRight,
    [LS_STEP_LEFT]    = Lara_State_StepLeft,
    [LS_HIT]          = Lara_State_Empty,
    [LS_SLIDE]        = Lara_State_Slide,
    [LS_BACK_JUMP]    = Lara_State_BackJump,
    [LS_RIGHT_JUMP]   = Lara_State_RightJump,
    [LS_LEFT_JUMP]    = Lara_State_LeftJump,
    [LS_UP_JUMP]      = Lara_State_UpJump,
    [LS_FALL_BACK]    = Lara_State_Fallback,
    [LS_HANG_LEFT]    = Lara_State_HangLeft,
    [LS_HANG_RIGHT]   = Lara_State_HangRight,
    [LS_SLIDE_BACK]   = Lara_State_SlideBack,
    [LS_SURF_TREAD]   = Lara_State_SurfTread,
    [LS_SURF_SWIM]    = Lara_State_SurfSwim,
    [LS_DIVE]         = Lara_State_Dive,
    [LS_PUSH_BLOCK]   = Lara_State_PushBlock,
    [LS_PULL_BLOCK]   = Lara_State_PushBlock,
    [LS_PP_READY]     = Lara_State_PPReady,
    [LS_PICKUP]       = Lara_State_Pickup,
    [LS_SWITCH_ON]    = Lara_State_SwitchOn,
    [LS_SWITCH_OFF]   = Lara_State_SwitchOn,
    [LS_USE_KEY]      = Lara_State_UseKey,
    [LS_USE_PUZZLE]   = Lara_State_UseKey,
    [LS_UW_DEATH]     = Lara_State_UWDeath,
    [LS_ROLL]         = Lara_State_Empty,
    [LS_SPECIAL]      = Lara_State_Special,
    [LS_SURF_BACK]    = Lara_State_SurfBack,
    [LS_SURF_LEFT]    = Lara_State_SurfLeft,
    [LS_SURF_RIGHT]   = Lara_State_SurfRight,
    [LS_USE_MIDAS]    = Lara_State_Empty,
    [LS_DIE_MIDAS]    = Lara_State_Empty,
    [LS_SWAN_DIVE]    = Lara_State_SwanDive,
    [LS_FAST_DIVE]    = Lara_State_FastDive,
    [LS_GYMNAST]      = Lara_State_Null,
    [LS_WATER_OUT]    = Lara_State_WaterOut,
    [LS_CLIMB_STANCE] = Lara_State_ClimbStance,
    [LS_CLIMBING]     = Lara_State_Climbing,
    [LS_CLIMB_LEFT]   = Lara_State_ClimbLeft,
    [LS_CLIMB_END]    = Lara_State_ClimbEnd,
    [LS_CLIMB_RIGHT]  = Lara_State_ClimbRight,
    [LS_CLIMB_DOWN]   = Lara_State_ClimbDown,
    [LS_LARA_TEST1]   = Lara_State_Empty,
    [LS_LARA_TEST2]   = Lara_State_Empty,
    [LS_LARA_TEST3]   = Lara_State_Empty,
    [LS_WADE]         = Lara_State_Wade,
    [LS_WATER_ROLL]   = Lara_State_UWTwist,
    [LS_FLARE_PICKUP] = Lara_State_PickupFlare,
    [LS_TWIST]        = Lara_State_Empty,
    [LS_KICK]         = Lara_State_Empty,
    [LS_ZIPLINE]      = Lara_State_Zipline,
    // clang-format on
};

static void (*m_ExtraControlRoutines[])(ITEM *item, COLL_INFO *coll) = {
    // clang-format off
    [LA_EXTRA_BREATH]      = Lara_State_Extra_Breath,
    [LA_EXTRA_PLUNGER]     = Lara_State_Empty,
    [LA_EXTRA_YETI_KILL]   = Lara_State_Extra_YetiKill,
    [LA_EXTRA_SHARK_KILL]  = Lara_State_Extra_SharkKill,
    [LA_EXTRA_AIRLOCK]     = Lara_State_Extra_Airlock,
    [LA_EXTRA_GONG_BONG]   = Lara_State_Extra_GongBong,
    [LA_EXTRA_TREX_KILL]   = Lara_State_Extra_DinoKill,
    [LA_EXTRA_PULL_DAGGER] = Lara_State_Extra_PullDagger,
    [LA_EXTRA_START_ANIM]  = Lara_State_Extra_StartAnim,
    [LA_EXTRA_START_HOUSE] = Lara_State_Extra_StartHouse,
    [LA_EXTRA_FINAL_ANIM]  = Lara_State_Extra_FinalAnim,
    // clang-format on
};

static void (*m_CollisionRoutines[])(ITEM *item, COLL_INFO *coll) = {
    // clang-format off
    [LS_WALK]         = Lara_Col_Walk,
    [LS_RUN]          = Lara_Col_Run,
    [LS_STOP]         = Lara_Col_Stop,
    [LS_FORWARD_JUMP] = Lara_Col_ForwardJump,
    [LS_POSE]         = Lara_Col_Land,
    [LS_FAST_BACK]    = Lara_Col_FastBack,
    [LS_TURN_RIGHT]   = Lara_Col_TurnRight,
    [LS_TURN_LEFT]    = Lara_Col_TurnLeft,
    [LS_DEATH]        = Lara_Col_Death,
    [LS_FAST_FALL]    = Lara_Col_FastFall,
    [LS_HANG]         = Lara_Col_Hang,
    [LS_REACH]        = Lara_Col_Reach,
    [LS_SPLAT]        = Lara_Col_Splat,
    [LS_TREAD]        = Lara_Col_Swim,
    [LS_LAND]         = Lara_Col_Land,
    [LS_COMPRESS]     = Lara_Col_Compress,
    [LS_BACK]         = Lara_Col_Back,
    [LS_SWIM]         = Lara_Col_Swim,
    [LS_GLIDE]        = Lara_Col_Swim,
    [LS_NULL]         = Lara_Col_Null,
    [LS_FAST_TURN]    = Lara_Col_Land,
    [LS_STEP_RIGHT]   = Lara_Col_StepRight,
    [LS_STEP_LEFT]    = Lara_Col_StepLeft,
    [LS_HIT]          = Lara_Col_Roll2,
    [LS_SLIDE]        = Lara_Col_Slide,
    [LS_BACK_JUMP]    = Lara_Col_BackJump,
    [LS_RIGHT_JUMP]   = Lara_Col_RightJump,
    [LS_LEFT_JUMP]    = Lara_Col_LeftJump,
    [LS_UP_JUMP]      = Lara_Col_UpJump,
    [LS_FALL_BACK]    = Lara_Col_Fallback,
    [LS_HANG_LEFT]    = Lara_Col_HangLeft,
    [LS_HANG_RIGHT]   = Lara_Col_HangRight,
    [LS_SLIDE_BACK]   = Lara_Col_SlideBack,
    [LS_SURF_TREAD]   = Lara_Col_SurfTread,
    [LS_SURF_SWIM]    = Lara_Col_SurfSwim,
    [LS_DIVE]         = Lara_Col_Swim,
    [LS_PUSH_BLOCK]   = Lara_Col_Null,
    [LS_PULL_BLOCK]   = Lara_Col_Null,
    [LS_PP_READY]     = Lara_Col_Null,
    [LS_PICKUP]       = Lara_Col_Null,
    [LS_SWITCH_ON]    = Lara_Col_Null,
    [LS_SWITCH_OFF]   = Lara_Col_Null,
    [LS_USE_KEY]      = Lara_Col_Null,
    [LS_USE_PUZZLE]   = Lara_Col_Null,
    [LS_UW_DEATH]     = Lara_Col_UWDeath,
    [LS_ROLL]         = Lara_Col_Roll,
    [LS_SPECIAL]      = Lara_Col_Empty,
    [LS_SURF_BACK]    = Lara_Col_SurfBack,
    [LS_SURF_LEFT]    = Lara_Col_SurfLeft,
    [LS_SURF_RIGHT]   = Lara_Col_SurfRight,
    [LS_USE_MIDAS]    = Lara_Col_Null,
    [LS_DIE_MIDAS]    = Lara_Col_Null,
    [LS_SWAN_DIVE]    = Lara_Col_SwanDive,
    [LS_FAST_DIVE]    = Lara_Col_FastDive,
    [LS_GYMNAST]      = Lara_Col_Null,
    [LS_WATER_OUT]    = Lara_Col_Null,
    [LS_CLIMB_STANCE] = Lara_Col_ClimbStance,
    [LS_CLIMBING]     = Lara_Col_Climbing,
    [LS_CLIMB_LEFT]   = Lara_Col_ClimbLeft,
    [LS_CLIMB_END]    = Lara_Col_Empty,
    [LS_CLIMB_RIGHT]  = Lara_Col_ClimbRight,
    [LS_CLIMB_DOWN]   = Lara_Col_ClimbDown,
    [LS_LARA_TEST1]   = Lara_Col_Empty,
    [LS_LARA_TEST2]   = Lara_Col_Empty,
    [LS_LARA_TEST3]   = Lara_Col_Empty,
    [LS_WADE]         = Lara_Col_Wade,
    [LS_WATER_ROLL]   = Lara_Col_Swim,
    [LS_FLARE_PICKUP] = Lara_Col_Null,
    [LS_TWIST]        = Lara_Col_Empty,
    [LS_KICK]         = Lara_Col_Empty,
    [LS_ZIPLINE]      = Lara_Col_Empty,
    // clang-format on
};

static SECTOR *M_GetCurrentSector(const ITEM *lara_item);

static SECTOR *M_GetCurrentSector(const ITEM *const lara_item)
{
    int16_t room_num = lara_item->room_num;
    return Room_GetSector(
        lara_item->pos.x, MAX_HEIGHT, lara_item->pos.z, &room_num);
}

void Lara_HandleAboveWater(ITEM *const item, COLL_INFO *const coll)
{
    coll->old.x = item->pos.x;
    coll->old.y = item->pos.y;
    coll->old.z = item->pos.z;
    coll->old_anim_state = item->current_anim_state;
    coll->old_anim_num = item->anim_num;
    coll->old_frame_num = item->frame_num;
    coll->radius = LARA_RADIUS;

    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_baddie_push = 1;
    coll->enable_spaz = 1;

    if (g_Input.look && !g_Lara.extra_anim && g_Lara.look) {
        Lara_LookLeftRight();
    } else {
        Lara_ResetLook();
    }
    g_Lara.look = 1;

    if (g_Lara.skidoo != NO_ITEM) {
        if (g_Items[g_Lara.skidoo].object_id == O_SKIDOO_FAST) {
            // TODO: make this g_Objects[O_SKIDOO_FAST].control
            if (Skidoo_Control()) {
                return;
            }
        } else {
            Gun_Control();
            return;
        }
    }

    if (g_Lara.extra_anim) {
        m_ExtraControlRoutines[item->current_anim_state](item, coll);
    } else {
        m_ControlRoutines[item->current_anim_state](item, coll);
    }

    if (item->rot.z < -LARA_LEAN_UNDO) {
        item->rot.z += LARA_LEAN_UNDO;
    } else if (item->rot.z > LARA_LEAN_UNDO) {
        item->rot.z -= LARA_LEAN_UNDO;
    } else {
        item->rot.z = 0;
    }

    if (g_Lara.turn_rate < -LARA_TURN_UNDO) {
        g_Lara.turn_rate += LARA_TURN_UNDO;
    } else if (g_Lara.turn_rate > LARA_TURN_UNDO) {
        g_Lara.turn_rate -= LARA_TURN_UNDO;
    } else {
        g_Lara.turn_rate = 0;
    }
    item->rot.y += g_Lara.turn_rate;

    Lara_Animate(item);

    const SECTOR *const sector = M_GetCurrentSector(item);
    if (!g_Lara.extra_anim && g_Lara.water_status != LWS_CHEAT) {
        Lara_BaddieCollision(item, coll);
        if (g_Lara.skidoo == NO_ITEM) {
            m_CollisionRoutines[item->current_anim_state](item, coll);
        }
    }

    Item_UpdateRoom(item, -LARA_HEIGHT / 2);
    Gun_Control();
    Room_TestSectorTrigger(item, sector);
}

void Lara_HandleSurface(ITEM *const item, COLL_INFO *const coll)
{
    g_Camera.target_elevation = -22 * PHD_DEGREE;

    coll->old.x = item->pos.x;
    coll->old.y = item->pos.y;
    coll->old.z = item->pos.z;
    coll->radius = LARA_RADIUS;

    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEP_L / 2;
    coll->bad_ceiling = 100;

    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_baddie_push = 0;
    coll->enable_spaz = 0;

    if (g_Input.look && g_Lara.look) {
        Lara_LookLeftRight();
    } else {
        Lara_ResetLook();
    }
    g_Lara.look = 1;

    m_ControlRoutines[item->current_anim_state](item, coll);

    if (item->rot.z > LARA_LEAN_UNDO_SURF) {
        item->rot.z -= LARA_LEAN_UNDO_SURF;
    } else if (item->rot.z < -LARA_LEAN_UNDO_SURF) {
        item->rot.z += LARA_LEAN_UNDO_SURF;
    } else {
        item->rot.z = 0;
    }

    if (g_Lara.current_active && g_Lara.water_status != LWS_CHEAT) {
        Lara_WaterCurrent(coll);
    }

    Lara_Animate(item);
    item->pos.x +=
        (item->fall_speed * Math_Sin(g_Lara.move_angle)) >> (W2V_SHIFT + 2);
    item->pos.z +=
        (item->fall_speed * Math_Cos(g_Lara.move_angle)) >> (W2V_SHIFT + 2);

    const SECTOR *const sector = M_GetCurrentSector(item);

    Lara_BaddieCollision(item, coll);

    if (g_Lara.skidoo == NO_ITEM) {
        m_CollisionRoutines[item->current_anim_state](item, coll);
    }

    Item_UpdateRoom(item, 100);
    Gun_Control();
    Room_TestSectorTrigger(item, sector);
}

void Lara_HandleUnderwater(ITEM *const item, COLL_INFO *const coll)
{
    coll->old.x = item->pos.x;
    coll->old.y = item->pos.y;
    coll->old.z = item->pos.z;
    coll->radius = LARA_RADIUS_UW;

    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -LARA_HEIGHT_UW;
    coll->bad_ceiling = LARA_HEIGHT_UW;

    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_baddie_push = 1;
    coll->enable_spaz = 0;

    if (g_Input.look && g_Lara.look) {
        Lara_LookLeftRight();
    } else {
        Lara_ResetLook();
    }
    g_Lara.look = 1;

    if (g_Lara.extra_anim) {
        m_ExtraControlRoutines[item->current_anim_state](item, coll);
    } else {
        m_ControlRoutines[item->current_anim_state](item, coll);
    }

    if (item->rot.z > LARA_LEAN_UNDO_UW) {
        item->rot.z -= LARA_LEAN_UNDO_UW;
    } else if (item->rot.z < -LARA_LEAN_UNDO_UW) {
        item->rot.z += LARA_LEAN_UNDO_UW;
    } else {
        item->rot.z = 0;
    }

    CLAMP(item->rot.x, -85 * PHD_DEGREE, 85 * PHD_DEGREE);
    CLAMP(item->rot.z, -LARA_LEAN_MAX_UW, LARA_LEAN_MAX_UW);

    if (g_Lara.turn_rate < -LARA_TURN_UNDO) {
        g_Lara.turn_rate += LARA_TURN_UNDO;
    } else if (g_Lara.turn_rate > LARA_TURN_UNDO) {
        g_Lara.turn_rate -= LARA_TURN_UNDO;
    } else {
        g_Lara.turn_rate = 0;
    }

    item->rot.y += g_Lara.turn_rate;
    if (g_Lara.current_active && g_Lara.water_status != LWS_CHEAT) {
        Lara_WaterCurrent(coll);
    }

    Lara_Animate(item);
    item->pos.y -=
        (item->fall_speed * Math_Sin(item->rot.x)) >> (W2V_SHIFT + 2);
    item->pos.x +=
        (Math_Cos(item->rot.x)
         * ((item->fall_speed * Math_Sin(item->rot.y)) >> (W2V_SHIFT + 2)))
        >> W2V_SHIFT;
    item->pos.z +=
        (Math_Cos(item->rot.x)
         * ((item->fall_speed * Math_Cos(item->rot.y)) >> (W2V_SHIFT + 2)))
        >> W2V_SHIFT;

    const SECTOR *const sector = M_GetCurrentSector(item);

    if (g_Lara.water_status != LWS_CHEAT && !g_Lara.extra_anim) {
        Lara_BaddieCollision(item, coll);
    }

    if (g_Lara.water_status == LWS_CHEAT) {
        if (m_OpenDoorsCheatCooldown) {
            m_OpenDoorsCheatCooldown--;
        } else if (g_InputDB.draw) {
            m_OpenDoorsCheatCooldown = FRAMES_PER_SECOND;
            Lara_Cheat_OpenNearestDoor();
        }
    }

    if (!g_Lara.extra_anim) {
        m_CollisionRoutines[item->current_anim_state](item, coll);
    }

    Item_UpdateRoom(item, 0);
    Gun_Control();
    Room_TestSectorTrigger(item, sector);
}

void Lara_Control(const int16_t item_num)
{
    ITEM *const item = g_LaraItem;

    if (g_InputDB.level_skip_cheat) {
        Lara_Cheat_EndLevel();
    }

    if (g_InputDB.item_cheat) {
        Lara_Cheat_GiveAllItems();
    }

    if (g_Lara.water_status != LWS_CHEAT && g_InputDB.fly_cheat) {
        Lara_Cheat_EnterFlyMode();
    }

    const bool room_submerged = g_Rooms[item->room_num].flags & RF_UNDERWATER;
    const int32_t water_depth = Lara_GetWaterDepth(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);
    const int32_t water_height = Room_GetWaterHeight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);
    const int32_t water_height_diff =
        water_height == NO_HEIGHT ? NO_HEIGHT : item->pos.y - water_height;

    g_Lara.water_surface_dist = -water_height_diff;

    if (g_Lara.skidoo == NO_ITEM && !g_Lara.extra_anim) {
        switch (g_Lara.water_status) {
        case LWS_ABOVE_WATER:
            if (water_height_diff == NO_HEIGHT
                || water_height_diff < LARA_WADE_DEPTH) {
                break;
            }

            if (water_depth <= LARA_SWIM_DEPTH - STEP_L) {
                if (water_height_diff > LARA_WADE_DEPTH) {
                    g_Lara.water_status = LWS_WADE;
                    if (!item->gravity) {
                        item->goal_anim_state = LS_STOP;
                    }
                }
            } else if (room_submerged) {
                g_Lara.air = LARA_MAX_AIR;
                g_Lara.water_status = LWS_UNDERWATER;
                item->gravity = 0;
                item->pos.y += 100;
                Item_UpdateRoom(item, 0);
                Sound_StopEffect(SFX_LARA_FALL);
                if (item->current_anim_state == LS_SWAN_DIVE) {
                    item->rot.x = -45 * PHD_DEGREE;
                    item->goal_anim_state = LS_DIVE;
                    Lara_Animate(item);
                    item->fall_speed *= 2;
                } else if (item->current_anim_state == LS_FAST_DIVE) {
                    item->rot.x = -85 * PHD_DEGREE;
                    item->goal_anim_state = LS_DIVE;
                    Lara_Animate(item);
                    item->fall_speed *= 2;
                } else {
                    item->rot.x = -45 * PHD_DEGREE;
                    Item_SwitchToAnim(item, LA_FREEFALL_TO_UNDERWATER, 0);
                    item->current_anim_state = LS_DIVE;
                    item->goal_anim_state = LS_SWIM;
                    item->fall_speed = item->fall_speed * 3 / 2;
                }
                g_Lara.torso_rot.y = 0;
                g_Lara.torso_rot.x = 0;
                g_Lara.head_rot.y = 0;
                g_Lara.head_rot.x = 0;
                Spawn_Splash(item);
            }
            break;

        case LWS_UNDERWATER:
            if (room_submerged) {
                break;
            }

            if (water_depth == NO_HEIGHT || ABS(water_height_diff) >= STEP_L) {
                g_Lara.water_status = LWS_ABOVE_WATER;
                Item_SwitchToAnim(item, LA_FALL_START, 0);
                item->goal_anim_state = LS_FORWARD_JUMP;
                item->current_anim_state = LS_FORWARD_JUMP;
                item->gravity = 1;
                item->speed = item->fall_speed / 4;
                item->fall_speed = 0;
                item->rot.x = 0;
                item->rot.z = 0;
                g_Lara.torso_rot.y = 0;
                g_Lara.torso_rot.x = 0;
                g_Lara.head_rot.y = 0;
                g_Lara.head_rot.x = 0;
            } else {
                g_Lara.water_status = LWS_SURFACE;
                Item_SwitchToAnim(item, LA_UNDERWATER_TO_ONWATER, 0);
                item->goal_anim_state = LS_SURF_TREAD;
                item->current_anim_state = LS_SURF_TREAD;
                item->fall_speed = 0;
                item->pos.y += 1 - water_height_diff;
                item->rot.z = 0;
                item->rot.x = 0;
                g_Lara.dive_count = 11;
                g_Lara.torso_rot.y = 0;
                g_Lara.torso_rot.x = 0;
                g_Lara.head_rot.y = 0;
                g_Lara.head_rot.x = 0;
                Item_UpdateRoom(item, -381);
                Sound_Effect(SFX_LARA_BREATH, &item->pos, SPM_ALWAYS);
            }
            break;

        case LWS_SURFACE:
            if (room_submerged) {
                break;
            }

            if (water_height_diff <= LARA_WADE_DEPTH) {
                g_Lara.water_status = LWS_ABOVE_WATER;
                Item_SwitchToAnim(item, LA_FALL_START, 0);
                item->goal_anim_state = LS_FORWARD_JUMP;
                item->current_anim_state = LS_FORWARD_JUMP;
                item->gravity = 1;
                item->speed = item->fall_speed / 4;
            } else {
                g_Lara.water_status = LWS_WADE;
                Item_SwitchToAnim(item, LA_STAND_IDLE, 0);
                item->current_anim_state = LS_STOP;
                item->goal_anim_state = LS_WADE;
                Item_Animate(item);
                item->fall_speed = 0;
            }
            item->rot.x = 0;
            item->rot.z = 0;
            g_Lara.torso_rot.y = 0;
            g_Lara.torso_rot.x = 0;
            g_Lara.head_rot.y = 0;
            g_Lara.head_rot.x = 0;
            break;

        case LWS_WADE:
            g_Camera.target_elevation = -22 * PHD_DEGREE;

            if (water_height_diff < LARA_WADE_DEPTH) {
                g_Lara.water_status = LWS_ABOVE_WATER;
                if (item->current_anim_state == LS_WADE) {
                    item->goal_anim_state = LS_RUN;
                }
            } else if (water_height_diff > 730) {
                g_Lara.water_status = LWS_SURFACE;
                item->pos.y += 1 - water_height_diff;

                LARA_ANIMATION anim_idx;
                switch (item->current_anim_state) {
                case LS_BACK:
                    item->goal_anim_state = LS_SURF_BACK;
                    anim_idx = LA_ONWATER_IDLE_TO_SWIM_BACK;
                    break;

                case LS_STEP_RIGHT:
                    item->goal_anim_state = LS_SURF_RIGHT;
                    anim_idx = LA_ONWATER_SWIM_RIGHT;
                    break;

                case LS_STEP_LEFT:
                    item->goal_anim_state = LS_SURF_LEFT;
                    anim_idx = LA_ONWATER_SWIM_LEFT;
                    break;

                default:
                    item->goal_anim_state = LS_SURF_SWIM;
                    anim_idx = LA_ONWATER_SWIM_FORWARD;
                    break;
                }
                item->current_anim_state = item->goal_anim_state;
                Item_SwitchToAnim(item, anim_idx, 0);

                item->rot.z = 0;
                item->rot.x = 0;
                item->gravity = 0;
                item->fall_speed = 0;
                g_Lara.dive_count = 0;
                g_Lara.torso_rot.y = 0;
                g_Lara.torso_rot.x = 0;
                g_Lara.head_rot.y = 0;
                g_Lara.head_rot.x = 0;
                Item_UpdateRoom(item, -LARA_HEIGHT / 2);
            }
            break;

        default:
            break;
        }
    }

    if (item->hit_points <= 0) {
        item->hit_points = -1;
        if (g_CurrentLevel == LV_GYM) {
            g_GymInvOpenEnabled = true;
        }
        if (!g_Lara.death_timer) {
            Music_Stop();
        }
        g_Lara.death_timer++;
        if (item->flags & IF_ONE_SHOT) {
            g_Lara.death_timer++;
            return;
        }
    } else if (g_GF_NoFloor && item->pos.y >= g_GF_NoFloor) {
        item->hit_points = -1;
        g_Lara.death_timer = 9 * FRAMES_PER_SECOND;
    }

    COLL_INFO coll;
    switch (g_Lara.water_status) {
    case LWS_ABOVE_WATER:
    case LWS_WADE:
        g_Lara.air = LARA_MAX_AIR;
        Lara_HandleAboveWater(item, &coll);
        break;

    case LWS_UNDERWATER:
        if (item->hit_points >= 0) {
            g_Lara.air--;
            if (g_Lara.air < 0) {
                g_Lara.air = -1;
                item->hit_points -= 5;
            }
        }
        Lara_HandleUnderwater(item, &coll);
        break;

    case LWS_SURFACE:
        if (item->hit_points >= 0) {
            g_Lara.air += 10;
            CLAMPG(g_Lara.air, LARA_MAX_AIR);
        }
        Lara_HandleSurface(item, &coll);
        break;

    case LWS_CHEAT:
        // TODO: make Lara immune to lava and flames
        item->hit_points = LARA_MAX_HITPOINTS;
        g_Lara.death_timer = 0;
        Lara_HandleUnderwater(item, &coll);
        if (g_Input.slow && !g_Input.look) {
            Lara_Cheat_ExitFlyMode();
        }
        break;

    default:
        break;
    }

    g_SaveGame.current_stats.distance += Math_Sqrt(
        SQUARE(item->pos.z - g_Lara.last_pos.z)
        + SQUARE(item->pos.y - g_Lara.last_pos.y)
        + SQUARE(item->pos.x - g_Lara.last_pos.x));

    g_Lara.last_pos = item->pos;
}

void Lara_ControlExtra(const int16_t item_num)
{
    Item_Animate(&g_Items[item_num]);
}

void Lara_Animate(ITEM *const item)
{
    item->frame_num++;

    const ANIM *anim = Item_GetAnim(item);
    if (anim->num_changes > 0 && Item_GetAnimChange(item, anim)) {
        anim = Item_GetAnim(item);
        item->current_anim_state = anim->current_anim_state;
    }

    if (item->frame_num > anim->frame_end) {
        if (anim->num_commands > 0) {
            const int16_t *cmd_ptr = Anim_GetCommand(anim->command_idx);
            for (int32_t i = 0; i < anim->num_commands; i++) {
                const int16_t cmd = *cmd_ptr++;

                switch (cmd) {
                case AC_MOVE_ORIGIN:
                    Item_Translate(item, cmd_ptr[0], cmd_ptr[1], cmd_ptr[2]);
                    cmd_ptr += 3;
                    break;

                case AC_JUMP_VELOCITY:
                    item->fall_speed = *cmd_ptr++;
                    item->speed = *cmd_ptr++;
                    item->gravity = 1;

                    if (g_Lara.calc_fall_speed) {
                        item->fall_speed = g_Lara.calc_fall_speed;
                        g_Lara.calc_fall_speed = 0;
                    }
                    break;

                case AC_ATTACK_READY:
                    if (g_Lara.gun_status != LGS_SPECIAL) {
                        g_Lara.gun_status = LGS_ARMLESS;
                    }
                    break;

                case AC_SOUND_FX:
                case AC_EFFECT:
                    cmd_ptr += 2;
                    break;

                default:
                    break;
                }
            }
        }

        item->anim_num = anim->jump_anim_num;
        item->frame_num = anim->jump_frame_num;
        anim = Item_GetAnim(item);
        item->current_anim_state = anim->current_anim_state;
    }

    if (anim->num_commands > 0) {
        const int16_t *cmd_ptr = Anim_GetCommand(anim->command_idx);
        for (int32_t i = 0; i < anim->num_commands; i++) {
            const int16_t cmd = *cmd_ptr++;

            switch (cmd) {
            case AC_MOVE_ORIGIN:
                cmd_ptr += 3;
                break;

            case AC_JUMP_VELOCITY:
                cmd_ptr += 2;
                break;

            case AC_SOUND_FX: {
                const int32_t frame = cmd_ptr[0];
                const SOUND_EFFECT_ID sound_id =
                    ANIM_CMD_PARAM_BITS(cmd_ptr[1]);
                const ANIM_COMMAND_ENVIRONMENT type =
                    ANIM_CMD_ENVIRONMENT_BITS(cmd_ptr[1]);
                cmd_ptr += 2;

                if (item->frame_num != frame) {
                    break;
                }

                if (type == ACE_ALL
                    || (type == ACE_LAND
                        && (g_Lara.water_surface_dist >= 0
                            || g_Lara.water_surface_dist == NO_HEIGHT))
                    || (type == ACE_WATER && g_Lara.water_surface_dist < 0
                        && g_Lara.water_surface_dist != NO_HEIGHT)) {
                    Sound_Effect(sound_id, &item->pos, SPM_ALWAYS);
                }
                break;
            }

            case AC_EFFECT:
                const int32_t frame = cmd_ptr[0];
                const int32_t action_id = ANIM_CMD_PARAM_BITS(cmd_ptr[1]);
                const ANIM_COMMAND_ENVIRONMENT type =
                    ANIM_CMD_ENVIRONMENT_BITS(cmd_ptr[1]);
                cmd_ptr += 2;

                if (item->frame_num != frame) {
                    break;
                }

                if (type == ACE_ALL
                    || (type == ACE_LAND
                        && (g_Lara.water_surface_dist >= 0
                            || g_Lara.water_surface_dist == NO_HEIGHT))
                    || (type == ACE_WATER && g_Lara.water_surface_dist < 0)) {
                    ItemAction_Run(action_id, item);
                }
                break;

            default:
                break;
            }
        }
    }

    if (item->gravity) {
        int32_t speed = anim->velocity
            + anim->acceleration * (item->frame_num - anim->frame_base - 1);
        item->speed -= (int16_t)(speed >> 16);
        speed += anim->acceleration;
        item->speed += (int16_t)(speed >> 16);

        item->fall_speed += item->fall_speed < FAST_FALL_SPEED ? GRAVITY : 1;
        item->pos.y += item->fall_speed;
    } else {
        int32_t speed = anim->velocity;
        if (anim->acceleration) {
            speed += anim->acceleration * (item->frame_num - anim->frame_base);
        }
        item->speed = (int16_t)(speed >> 16);
    }

    item->pos.x += (item->speed * Math_Sin(g_Lara.move_angle)) >> W2V_SHIFT;
    item->pos.z += (item->speed * Math_Cos(g_Lara.move_angle)) >> W2V_SHIFT;
}

void Lara_UseItem(const GAME_OBJECT_ID object_id)
{
    ITEM *const item = g_LaraItem;

    switch (object_id) {
    case O_PISTOL_ITEM:
    case O_PISTOL_OPTION:
        g_Lara.request_gun_type = LGT_PISTOLS;
        break;

    case O_SHOTGUN_ITEM:
    case O_SHOTGUN_OPTION:
        g_Lara.request_gun_type = LGT_SHOTGUN;
        break;

    case O_MAGNUM_ITEM:
    case O_MAGNUM_OPTION:
        g_Lara.request_gun_type = LGT_MAGNUMS;
        break;

    case O_UZI_ITEM:
    case O_UZI_OPTION:
        g_Lara.request_gun_type = LGT_UZIS;
        break;

    case O_HARPOON_ITEM:
    case O_HARPOON_OPTION:
        g_Lara.request_gun_type = LGT_HARPOON;
        break;

    case O_M16_ITEM:
    case O_M16_OPTION:
        g_Lara.request_gun_type = LGT_M16;
        break;

    case O_GRENADE_ITEM:
    case O_GRENADE_OPTION:
        g_Lara.request_gun_type = LGT_GRENADE;
        break;

    case O_SMALL_MEDIPACK_ITEM:
    case O_SMALL_MEDIPACK_OPTION:
        if (item->hit_points > 0 && item->hit_points < LARA_MAX_HITPOINTS) {
            item->hit_points += LARA_MAX_HITPOINTS / 2;
            CLAMPG(item->hit_points, LARA_MAX_HITPOINTS);
            Inv_RemoveItem(O_SMALL_MEDIPACK_ITEM);
            Sound_Effect(SFX_MENU_MEDI, NULL, SPM_ALWAYS);
            g_SaveGame.current_stats.medipacks++;
        }
        break;

    case O_LARGE_MEDIPACK_ITEM:
    case O_LARGE_MEDIPACK_OPTION:
        if (item->hit_points > 0 && item->hit_points < LARA_MAX_HITPOINTS) {
            item->hit_points = LARA_MAX_HITPOINTS;
            Inv_RemoveItem(O_LARGE_MEDIPACK_ITEM);
            Sound_Effect(SFX_MENU_MEDI, NULL, SPM_ALWAYS);
            g_SaveGame.current_stats.medipacks += 2;
        }
        break;

    case O_FLARES_ITEM:
    case O_FLARES_OPTION:
        g_Lara.request_gun_type = LGT_FLARE;
        break;

    default:
        break;
    }
}

void Lara_InitialiseLoad(const int16_t item_num)
{
    g_Lara.item_num = item_num;
    g_LaraItem = &g_Items[item_num];
}

void Lara_Initialise(const GAME_FLOW_LEVEL_TYPE type)
{
    ITEM *const item = g_LaraItem;

    item->data = &g_Lara;
    item->collidable = 0;
    item->hit_points = LARA_MAX_HITPOINTS;

    g_Lara.hit_direction = -1;
    g_Lara.skidoo = NO_ITEM;
    g_Lara.weapon_item = NO_ITEM;
    g_Lara.calc_fall_speed = 0;
    g_Lara.climb_status = 0;
    g_Lara.pose_count = 0;
    g_Lara.hit_frame = 0;
    g_Lara.air = LARA_MAX_AIR;
    g_Lara.dive_count = 0;
    g_Lara.death_timer = 0;
    g_Lara.current_active = 0;
    g_Lara.spaz_effect_count = 0;
    g_Lara.flare_age = 0;
    g_Lara.back_gun = 0;
    g_Lara.flare_frame = 0;
    g_Lara.flare_control_left = 0;
    g_Lara.flare_control_right = 0;
    g_Lara.extra_anim = 0;
    g_Lara.look = 1;
    g_Lara.burn = 0;
    g_Lara.water_surface_dist = 100;
    g_Lara.last_pos = item->pos;
    g_Lara.spaz_effect = NULL;
    g_Lara.mesh_effects = 0;
    g_Lara.target = NULL;
    g_Lara.turn_rate = 0;
    g_Lara.move_angle = 0;
    g_Lara.head_rot.x = 0;
    g_Lara.head_rot.y = 0;
    g_Lara.head_rot.z = 0;
    g_Lara.torso_rot.x = 0;
    g_Lara.torso_rot.y = 0;
    g_Lara.torso_rot.z = 0;
    g_Lara.left_arm.flash_gun = 0;
    g_Lara.right_arm.flash_gun = 0;
    g_Lara.left_arm.lock = 0;
    g_Lara.right_arm.lock = 0;
    g_Lara.creature = NULL;

    if (type == GFL_NORMAL && g_GF_LaraStartAnim) {
        g_Lara.water_status = LWS_ABOVE_WATER;
        g_Lara.gun_status = LGS_HANDS_BUSY;
        Item_SwitchToObjAnim(item, LA_EXTRA_BREATH, 0, O_LARA_EXTRA);
        item->current_anim_state = LA_EXTRA_BREATH;
        item->goal_anim_state = g_GF_LaraStartAnim;
        Lara_Animate(item);
        g_Lara.extra_anim = 1;
        g_Camera.type = CAM_CINEMATIC;
        g_CineFrameIdx = 0;
        g_CinePos.pos = item->pos;
        g_CinePos.rot = item->rot;
    } else if ((g_Rooms[item->room_num].flags & RF_UNDERWATER)) {
        g_Lara.water_status = LWS_UNDERWATER;
        item->fall_speed = 0;
        item->goal_anim_state = LS_TREAD;
        item->current_anim_state = LS_TREAD;
        Item_SwitchToAnim(item, LA_UNDERWATER_IDLE, 0);
    } else {
        g_Lara.water_status = LWS_ABOVE_WATER;
        item->goal_anim_state = LS_STOP;
        item->current_anim_state = LS_STOP;
        Item_SwitchToAnim(item, LA_STAND_STILL, 0);
    }

    if (type == GFL_CUTSCENE) {
        for (int32_t i = 0; i < LM_NUMBER_OF; i++) {
            g_Lara.mesh_ptrs[i] = g_Meshes[g_Objects[O_LARA].mesh_idx + i];
        }

        g_Lara.mesh_ptrs[LM_THIGH_L] =
            g_Meshes[g_Objects[O_LARA_PISTOLS].mesh_idx + LM_THIGH_L];
        g_Lara.mesh_ptrs[LM_THIGH_R] =
            g_Meshes[g_Objects[O_LARA_PISTOLS].mesh_idx + LM_THIGH_R];
        g_Lara.gun_status = LGS_ARMLESS;
    } else {
        Lara_InitialiseInventory(g_CurrentLevel);
    }
}

void Lara_InitialiseInventory(const int32_t level_num)
{
    Inv_RemoveAllItems();

    START_INFO *const start = &g_SaveGame.start[level_num];
    if (g_GF_RemoveWeapons) {
        start->has_pistols = 0;
        start->has_magnums = 0;
        start->has_uzis = 0;
        start->has_shotgun = 0;
        start->has_m16 = 0;
        start->has_grenade = 0;
        start->has_harpoon = 0;
        start->gun_type = LGT_UNARMED;
        start->gun_status = LGS_ARMLESS;
        g_GF_RemoveWeapons = false;
    }

    if (g_GF_RemoveAmmo) {
        start->m16_ammo = 0;
        start->grenade_ammo = 0;
        start->harpoon_ammo = 0;
        start->shotgun_ammo = 0;
        start->uzi_ammo = 0;
        start->magnum_ammo = 0;
        start->pistol_ammo = 0;
        start->flares = 0;
        start->large_medipacks = 0;
        start->small_medipacks = 0;
        g_GF_RemoveAmmo = false;
    }

    Inv_AddItem(O_COMPASS_ITEM);

    g_Lara.pistol_ammo.ammo = 1000;
    if (start->has_pistols) {
        Inv_AddItem(O_PISTOL_ITEM);
    }

    if (start->has_magnums) {
        Inv_AddItem(O_MAGNUM_ITEM);
        g_Lara.magnum_ammo.ammo = start->magnum_ammo;
        Item_GlobalReplace(O_MAGNUM_ITEM, O_MAGNUM_AMMO_ITEM);
    } else {
        Inv_AddItemNTimes(O_MAGNUM_AMMO_ITEM, start->magnum_ammo / 40);
        g_Lara.magnum_ammo.ammo = 0;
    }

    if (start->has_uzis) {
        Inv_AddItem(O_UZI_ITEM);
        g_Lara.uzi_ammo.ammo = start->uzi_ammo;
        Item_GlobalReplace(O_UZI_ITEM, O_UZI_AMMO_ITEM);
    } else {
        Inv_AddItemNTimes(O_UZI_AMMO_ITEM, start->uzi_ammo / 80);
        g_Lara.uzi_ammo.ammo = 0;
    }

    if (start->has_shotgun) {
        Inv_AddItem(O_SHOTGUN_ITEM);
        g_Lara.shotgun_ammo.ammo = start->shotgun_ammo;
        Item_GlobalReplace(O_SHOTGUN_ITEM, O_SHOTGUN_AMMO_ITEM);
    } else {
        Inv_AddItemNTimes(O_SHOTGUN_AMMO_ITEM, start->shotgun_ammo / 12);
        g_Lara.shotgun_ammo.ammo = 0;
    }

    if (start->has_m16) {
        Inv_AddItem(O_M16_ITEM);
        g_Lara.m16_ammo.ammo = start->m16_ammo;
        Item_GlobalReplace(O_M16_ITEM, O_M16_AMMO_ITEM);
    } else {
        Inv_AddItemNTimes(O_M16_AMMO_ITEM, start->m16_ammo / 40);
        g_Lara.m16_ammo.ammo = 0;
    }

    if (start->has_grenade) {
        Inv_AddItem(O_GRENADE_ITEM);
        g_Lara.grenade_ammo.ammo = start->grenade_ammo;
        Item_GlobalReplace(O_GRENADE_ITEM, O_GRENADE_AMMO_ITEM);
    } else {
        Inv_AddItemNTimes(O_GRENADE_AMMO_ITEM, start->grenade_ammo / 2);
        g_Lara.grenade_ammo.ammo = 0;
    }

    if (start->has_harpoon) {
        Inv_AddItem(O_HARPOON_ITEM);
        g_Lara.harpoon_ammo.ammo = start->harpoon_ammo;
        Item_GlobalReplace(O_HARPOON_ITEM, O_HARPOON_AMMO_ITEM);
    } else {
        Inv_AddItemNTimes(O_HARPOON_AMMO_ITEM, start->harpoon_ammo / 3);
        g_Lara.harpoon_ammo.ammo = 0;
    }

    Inv_AddItemNTimes(O_FLARE_ITEM, start->flares);
    Inv_AddItemNTimes(O_SMALL_MEDIPACK_ITEM, start->small_medipacks);
    Inv_AddItemNTimes(O_LARGE_MEDIPACK_ITEM, start->large_medipacks);

    g_Lara.gun_status = LGS_ARMLESS;
    g_Lara.last_gun_type = start->gun_type;
    g_Lara.gun_type = g_Lara.last_gun_type;
    g_Lara.request_gun_type = g_Lara.last_gun_type;

    Lara_InitialiseMeshes(level_num);
    Gun_InitialiseNewWeapon();
}

void Lara_InitialiseMeshes(const int32_t level_num)
{
    for (int32_t i = 0; i < LM_NUMBER_OF; i++) {
        g_Lara.mesh_ptrs[i] = g_Meshes[g_Objects[O_LARA].mesh_idx + i];
    }

    const START_INFO *const start = &g_SaveGame.start[level_num];

    GAME_OBJECT_ID holster_object_id = NO_OBJECT;
    if (start->gun_type != LGT_UNARMED) {
        if (start->gun_type == LGT_MAGNUMS) {
            holster_object_id = O_LARA_MAGNUMS;
        } else if (start->gun_type == LGT_UZIS) {
            holster_object_id = O_LARA_UZIS;
        } else {
            holster_object_id = O_LARA_PISTOLS;
        }
    }

    if (holster_object_id != NO_OBJECT) {
        g_Lara.mesh_ptrs[LM_THIGH_L] =
            g_Meshes[g_Objects[holster_object_id].mesh_idx + LM_THIGH_L];
        g_Lara.mesh_ptrs[LM_THIGH_R] =
            g_Meshes[g_Objects[holster_object_id].mesh_idx + LM_THIGH_R];
    }

    if (start->gun_type == LGT_FLARE) {
        g_Lara.mesh_ptrs[LM_HAND_L] =
            g_Meshes[g_Objects[O_LARA_FLARE].mesh_idx + LM_HAND_L];
    }

    switch (start->gun_type) {
    case LGT_M16:
        g_Lara.back_gun = O_LARA_M16;
        return;

    case LGT_GRENADE:
        g_Lara.back_gun = O_LARA_GRENADE;
        return;

    case LGT_HARPOON:
        g_Lara.back_gun = O_LARA_HARPOON;
        return;
    }

    if (start->has_shotgun) {
        g_Lara.back_gun = O_LARA_SHOTGUN;
    } else if (start->has_m16) {
        g_Lara.back_gun = O_LARA_M16;
    } else if (start->has_grenade) {
        g_Lara.back_gun = O_LARA_GRENADE;
    } else if (start->has_harpoon) {
        g_Lara.back_gun = O_LARA_HARPOON;
    }
}

void Lara_SwapSingleMesh(const LARA_MESH mesh, const GAME_OBJECT_ID object_id)
{
    g_Lara.mesh_ptrs[mesh] = g_Meshes[g_Objects[object_id].mesh_idx + mesh];
}

void Lara_GetOffVehicle(void)
{
    if (g_Lara.skidoo != NO_ITEM) {
        ITEM *const vehicle = &g_Items[g_Lara.skidoo];
        Item_SwitchToAnim(vehicle, 0, 0);
        g_Lara.skidoo = NO_ITEM;

        g_LaraItem->current_anim_state = LS_STOP;
        g_LaraItem->goal_anim_state = LS_STOP;
        Item_SwitchToAnim(g_LaraItem, LA_STAND_STILL, 0);

        g_LaraItem->rot.x = 0;
        g_LaraItem->rot.z = 0;
    }
}

int16_t Lara_GetNearestEnemy(void)
{
    if (g_LaraItem == NULL) {
        return NO_ITEM;
    }

    int32_t best_distance = -1;
    int16_t best_item_num = NO_ITEM;
    int16_t item_num = g_NextItemActive;
    while (item_num != NO_ITEM) {
        const ITEM *const item = &g_Items[item_num];

        if (Creature_IsEnemy(item)) {
            const int32_t distance = Item_GetDistance(item, &g_LaraItem->pos);
            if (best_item_num == NO_ITEM || distance < best_distance) {
                best_item_num = item_num;
                best_distance = distance;
            }
        }

        item_num = item->next_active;
    }

    return best_item_num;
}

void Lara_TakeDamage(const int16_t damage, const bool hit_status)
{
    Item_TakeDamage(g_LaraItem, damage, hit_status);
}
