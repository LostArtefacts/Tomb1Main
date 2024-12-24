#include "game/item_actions.h"

#include "decomp/stats.h"
#include "game/collide.h"
#include "game/lara/hair.h"
#include "game/music.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "game/spawn.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/utils.h>

typedef void (*M_FUNC)(ITEM *item);

static void M_Turn180(ITEM *item);
static void M_FloorShake(ITEM *item);
static void M_LaraNormal(ITEM *item);
static void M_Bubbles(ITEM *item);
static void M_FinishLevel(ITEM *item);
static void M_Flood(ITEM *item);
static void M_Chandelier(ITEM *item);
static void M_Rubble(ITEM *item);
static void M_Piston(ITEM *item);
static void M_Curtain(ITEM *item);
static void M_SetChange(ITEM *item);
static void M_Explosion(ITEM *item);
static void M_LaraHandsFree(ITEM *item);
static void M_FlipMap(ITEM *item);
static void M_LaraDrawRightGun(ITEM *item);
static void M_LaraDrawLeftGun(ITEM *item);
static void M_SwapMeshesWithMeshSwap1(ITEM *item);
static void M_SwapMeshesWithMeshSwap2(ITEM *item);
static void M_SwapMeshesWithMeshSwap3(ITEM *item);
static void M_InvisibilityOn(ITEM *item);
static void M_InvisibilityOff(ITEM *item);
static void M_DynamicLightOn(ITEM *item);
static void M_DynamicLightOff(ITEM *item);
static void M_Statue(ITEM *item);
static void M_ResetHair(ITEM *item);
static void M_Boiler(ITEM *item);
static void M_AssaultReset(ITEM *item);
static void M_AssaultStop(ITEM *item);
static void M_AssaultStart(ITEM *item);
static void M_AssaultFinished(ITEM *item);

static M_FUNC m_Actions[] = {
    // clang-format off
    [ITEM_ACTION_TURN_180]                     = M_Turn180,
    [ITEM_ACTION_FLOOR_SHAKE]                  = M_FloorShake,
    [ITEM_ACTION_LARA_NORMAL]                  = M_LaraNormal,
    [ITEM_ACTION_BUBBLES]                      = M_Bubbles,
    [ITEM_ACTION_FINISH_LEVEL]                 = M_FinishLevel,
    [ITEM_ACTION_FLOOD]                        = M_Flood,
    [ITEM_ACTION_CHANDELIER]                   = M_Chandelier,
    [ITEM_ACTION_RUBBLE]                       = M_Rubble,
    [ITEM_ACTION_PISTON]                       = M_Piston,
    [ITEM_ACTION_CURTAIN]                      = M_Curtain,
    [ITEM_ACTION_SET_CHANGE]                   = M_SetChange,
    [ITEM_ACTION_EXPLOSION]                    = M_Explosion,
    [ITEM_ACTION_LARA_HANDS_FREE]              = M_LaraHandsFree,
    [ITEM_ACTION_FLIPMAP]                      = M_FlipMap,
    [ITEM_ACTION_LARA_DRAW_RIGHT_GUN]          = M_LaraDrawRightGun,
    [ITEM_ACTION_LARA_DRAW_LEFT_GUN]           = M_LaraDrawLeftGun,
    [ITEM_ACTION_SWAP_MESHES_WITH_MESH_SWAP_1] = M_SwapMeshesWithMeshSwap1,
    [ITEM_ACTION_SWAP_MESHES_WITH_MESH_SWAP_2] = M_SwapMeshesWithMeshSwap2,
    [ITEM_ACTION_SWAP_MESHES_WITH_MESH_SWAP_3] = M_SwapMeshesWithMeshSwap3,
    [ITEM_ACTION_INVISIBILITY_ON]              = M_InvisibilityOn,
    [ITEM_ACTION_INVISIBILITY_OFF]             = M_InvisibilityOff,
    [ITEM_ACTION_DYNAMIC_LIGHT_ON]             = M_DynamicLightOn,
    [ITEM_ACTION_DYNAMIC_LIGHT_OFF]            = M_DynamicLightOff,
    [ITEM_ACTION_STATUE]                       = M_Statue,
    [ITEM_ACTION_RESET_HAIR]                   = M_ResetHair,
    [ITEM_ACTION_BOILER]                       = M_Boiler,
    [ITEM_ACTION_ASSAULT_RESET]                = M_AssaultReset,
    [ITEM_ACTION_ASSAULT_STOP]                 = M_AssaultStop,
    [ITEM_ACTION_ASSAULT_START]                = M_AssaultStart,
    [ITEM_ACTION_ASSAULT_FINISHED]             = M_AssaultFinished,
    // clang-format on
};

void M_Bubbles(ITEM *const item)
{
    const int32_t count = (Random_GetDraw() * 3) / 0x8000;
    if (count == 0) {
        return;
    }

    Sound_Effect(SFX_LARA_BUBBLES, &item->pos, SPM_UNDERWATER);

    XYZ_32 offset = { .x = 0, .y = 0, .z = 50 };
    Collide_GetJointAbsPosition(item, &offset, LM_HEAD);
    for (int32_t i = 0; i < count; i++) {
        Spawn_Bubble(&offset, item->room_num);
    }
}

void M_LaraHandsFree(ITEM *const item)
{
    g_Lara.gun_status = LGS_ARMLESS;
}

void M_FinishLevel(ITEM *const item)
{
    g_LevelComplete = true;
}

void M_Turn180(ITEM *const item)
{
    item->rot.x = -item->rot.x;
    item->rot.y += PHD_180;
}

void M_FloorShake(ITEM *const item)
{
    const int32_t max_dist = WALL_L * 16; // = 0x4000
    const int32_t max_bounce = 100;

    const int32_t dx = item->pos.x - g_Camera.pos.pos.x;
    const int32_t dy = item->pos.y - g_Camera.pos.pos.y;
    const int32_t dz = item->pos.z - g_Camera.pos.pos.z;
    const int32_t dist = SQUARE(dz) + SQUARE(dy) + SQUARE(dx);

    if (ABS(dx) < max_dist && ABS(dy) < max_dist && ABS(dz) < max_dist) {
        g_Camera.bounce =
            max_bounce * (SQUARE(WALL_L) - dist / 256) / SQUARE(WALL_L);
    }
}

void M_LaraNormal(ITEM *const item)
{
    item->current_anim_state = LS_STOP;
    item->goal_anim_state = LS_STOP;
    item->anim_num = LA_STAND_STILL;
    item->frame_num = g_Anims[item->anim_num].frame_base;
    g_Camera.type = CAM_CHASE;
    Output_AlterFOV(GAME_FOV * PHD_DEGREE);
}

void M_Boiler(ITEM *const item)
{
    Sound_Effect(SFX_UNKNOWN_1, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void M_Flood(ITEM *const item)
{
    if (g_FlipTimer > 4 * FRAMES_PER_SECOND) {
        g_FlipEffect = -1;
        g_FlipTimer++;
        return;
    }

    XYZ_32 pos = {
        .x = g_LaraItem->pos.x,
        .y = g_Camera.target.pos.y,
        .z = g_LaraItem->pos.z,
    };

    if (g_FlipTimer >= FRAMES_PER_SECOND) {
        pos.y += 100 * (g_FlipTimer - FRAMES_PER_SECOND);
    } else {
        pos.y += 100 * (FRAMES_PER_SECOND - g_FlipTimer);
    }

    Sound_Effect(SFX_WATERFALL_LOOP, &pos, SPM_NORMAL);
    g_FlipTimer++;
}

void M_Rubble(ITEM *const item)
{
    Sound_Effect(SFX_MASSIVE_CRASH, NULL, SPM_NORMAL);
    g_Camera.bounce = -350;
    g_FlipEffect = -1;
}

void M_Chandelier(ITEM *const item)
{
    Sound_Effect(SFX_CHAIN_PULLEY, NULL, SPM_NORMAL);
    g_FlipTimer++;
    if (g_FlipTimer > FRAMES_PER_SECOND) {
        g_FlipEffect = -1;
    }
}

void M_Explosion(ITEM *const item)
{
    Sound_Effect(SFX_EXPLOSION_1, NULL, SPM_NORMAL);
    g_Camera.bounce = -75;
    g_FlipEffect = -1;
}

void M_Piston(ITEM *const item)
{
    Sound_Effect(SFX_PULLEY_CRANE, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void M_Curtain(ITEM *const item)
{
    Sound_Effect(SFX_CURTAIN, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void M_Statue(ITEM *const item)
{
    Sound_Effect(SFX_STONE_DOOR_SLIDE, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void M_SetChange(ITEM *const item)
{
    Sound_Effect(SFX_STAGE_BACKDROP, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void M_FlipMap(ITEM *const item)
{
    Room_FlipMap();
}

void M_LaraDrawRightGun(ITEM *const item)
{
    const OBJECT *const obj = Object_GetObject(O_LARA_PISTOLS);
    int16_t *tmp;
    SWAP(
        g_Lara.mesh_ptrs[LM_THIGH_R], g_Meshes[obj->mesh_idx + LM_THIGH_R],
        tmp);
    SWAP(g_Lara.mesh_ptrs[LM_HAND_R], g_Meshes[obj->mesh_idx + LM_HAND_R], tmp);
}

void M_LaraDrawLeftGun(ITEM *const item)
{
    const OBJECT *const obj = Object_GetObject(O_LARA_PISTOLS);
    int16_t *tmp;
    SWAP(
        g_Lara.mesh_ptrs[LM_THIGH_L], g_Meshes[obj->mesh_idx + LM_THIGH_L],
        tmp);
    SWAP(g_Lara.mesh_ptrs[LM_HAND_L], g_Meshes[obj->mesh_idx + LM_HAND_L], tmp);
}

void M_SwapMeshesWithMeshSwap1(ITEM *const item)
{
    const OBJECT *const obj_1 = Object_GetObject(item->object_id);
    const OBJECT *const obj_2 = Object_GetObject(O_MESH_SWAP_1);
    for (int32_t mesh_idx = 0; mesh_idx < obj_1->mesh_count; mesh_idx++) {
        int16_t *tmp;
        SWAP(
            g_Meshes[obj_1->mesh_idx + mesh_idx],
            g_Meshes[obj_2->mesh_idx + mesh_idx], tmp);
    }
}

void M_SwapMeshesWithMeshSwap2(ITEM *const item)
{
    const OBJECT *const obj_1 = Object_GetObject(item->object_id);
    const OBJECT *const obj_2 = Object_GetObject(O_MESH_SWAP_2);
    for (int32_t mesh_idx = 0; mesh_idx < obj_1->mesh_count; mesh_idx++) {
        int16_t *tmp;
        SWAP(
            g_Meshes[obj_1->mesh_idx + mesh_idx],
            g_Meshes[obj_2->mesh_idx + mesh_idx], tmp);
    }
}

void M_SwapMeshesWithMeshSwap3(ITEM *const item)
{
    OBJECT *const obj_1 = Object_GetObject(item->object_id);
    OBJECT *const obj_2 = Object_GetObject(O_LARA_SWAP);
    for (int32_t mesh_idx = 0; mesh_idx < obj_1->mesh_count; mesh_idx++) {
        int16_t *tmp;
        SWAP(
            g_Meshes[obj_1->mesh_idx + mesh_idx],
            g_Meshes[obj_2->mesh_idx + mesh_idx], tmp);
        if (item == g_LaraItem) {
            g_Lara.mesh_ptrs[mesh_idx] = g_Meshes[obj_1->mesh_idx + mesh_idx];
        }
    }
}

void M_InvisibilityOn(ITEM *const item)
{
    item->status = IS_INVISIBLE;
}

void M_InvisibilityOff(ITEM *const item)
{
    item->status = IS_ACTIVE;
}

void M_DynamicLightOn(ITEM *const item)
{
    item->dynamic_light = 1;
}

void M_DynamicLightOff(ITEM *const item)
{
    item->dynamic_light = 0;
}

void M_ResetHair(ITEM *const item)
{
    Lara_Hair_Initialise();
}

void M_AssaultStart(ITEM *const item)
{
    g_SaveGame.statistics.timer = 0;
    g_IsAssaultTimerActive = 1;
    g_IsAssaultTimerDisplay = 1;
    g_FlipEffect = -1;
    Stats_StartTimer();
}

void M_AssaultStop(ITEM *const item)
{
    g_IsAssaultTimerActive = false;
    g_IsAssaultTimerDisplay = true;
    g_FlipEffect = -1;
}

void M_AssaultReset(ITEM *const item)
{
    g_IsAssaultTimerActive = false;
    g_IsAssaultTimerDisplay = false;
    g_FlipEffect = -1;
}

void M_AssaultFinished(ITEM *const item)
{
    if (g_IsAssaultTimerActive) {
        AddAssaultTime(g_SaveGame.statistics.timer);

        if ((int32_t)g_AssaultBestTime < 0) {
            if (g_SaveGame.statistics.timer < 100 * FRAMES_PER_SECOND) {
                // "Gosh! That was my best time yet!"
                Music_Legacy_Play(MX_GYM_HINT_15, false);
                g_AssaultBestTime = g_SaveGame.statistics.timer;
            } else {
                // "Congratulations! You did it! But perhaps I could've been
                // faster."
                Music_Legacy_Play(MX_GYM_HINT_17, false);
                g_AssaultBestTime = 100 * FRAMES_PER_SECOND;
            }
        } else if (g_SaveGame.statistics.timer < g_AssaultBestTime) {
            // "Gosh! That was my best time yet!"
            Music_Legacy_Play(MX_GYM_HINT_15, false);
            g_AssaultBestTime = g_SaveGame.statistics.timer;
        } else if (
            g_SaveGame.statistics.timer
            < g_AssaultBestTime + 5 * FRAMES_PER_SECOND) {
            // "Almost. Perhaps another try and I might beat it."
            Music_Legacy_Play(MX_GYM_HINT_16, false);
        } else {
            // "Great. But nowhere near my best time."
            Music_Legacy_Play(MX_GYM_HINT_14, false);
        }

        g_IsAssaultTimerActive = false;
    }

    g_FlipEffect = -1;
}

void ItemAction_Run(int16_t action_id, ITEM *item)
{
    if (m_Actions[action_id] != NULL) {
        m_Actions[action_id](item);
    }
}

void ItemAction_RunActive(void)
{
    if (g_FlipEffect != -1) {
        ItemAction_Run(g_FlipEffect, NULL);
    }
}
