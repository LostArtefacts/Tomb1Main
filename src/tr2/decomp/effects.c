#include "decomp/effects.h"

#include "game/collide.h"
#include "game/effects.h"
#include "game/math.h"
#include "game/matrix.h"
#include "game/objects/effects/missile_common.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/funcs.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#define BARTOLI_LIGHT_RANGE (5 * WALL_L) // = 5120

int32_t __cdecl Effect_ExplodingDeath(
    const int16_t item_num, const int32_t mesh_bits, const int16_t damage)
{
    ITEM *const item = &g_Items[item_num];
    const OBJECT *const object = &g_Objects[item->object_id];

    Output_CalculateLight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);

    const FRAME_INFO *const best_frame = Item_GetBestFrame(item);

    Matrix_PushUnit();
    g_MatrixPtr->_03 = 0;
    g_MatrixPtr->_13 = 0;
    g_MatrixPtr->_23 = 0;
    g_MatrixPtr->_23 = 0;

    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);
    Matrix_TranslateRel(
        best_frame->offset.x, best_frame->offset.y, best_frame->offset.z);

    const int16_t *mesh_rots = best_frame->mesh_rots;
    Matrix_RotYXZsuperpack(&mesh_rots, 0);

    // main mesh
    int32_t bit = 1;
    if ((mesh_bits & bit) && (item->mesh_bits & bit)) {
        const int16_t fx_num = Effect_Create(item->room_num);
        if (fx_num != NO_ITEM) {
            FX *const fx = &g_Effects[fx_num];
            fx->pos.x = item->pos.x + (g_MatrixPtr->_03 >> W2V_SHIFT);
            fx->pos.y = item->pos.y + (g_MatrixPtr->_13 >> W2V_SHIFT);
            fx->pos.z = item->pos.z + (g_MatrixPtr->_23 >> W2V_SHIFT);
            fx->rot.y = (Random_GetControl() - 0x4000) * 2;
            fx->room_num = item->room_num;
            fx->speed = Random_GetControl() >> 8;
            fx->fall_speed = -Random_GetControl() >> 8;
            fx->counter = damage;
            fx->object_id = O_BODY_PART;
            fx->frame_num = object->mesh_idx;
            fx->shade = g_LsAdder - 0x300;
        }
        item->mesh_bits &= ~bit;
    }

    // additional meshes
    const int32_t *bone = &g_AnimBones[object->bone_idx];
    const int16_t *extra_rotation = (int16_t *)item->data;
    for (int32_t i = 1; i < object->mesh_count; i++) {
        uint32_t bone_flags = *bone++;
        if (bone_flags & BF_MATRIX_POP) {
            Matrix_Pop();
        }
        if (bone_flags & BF_MATRIX_PUSH) {
            Matrix_Push();
        }

        Matrix_TranslateRel(bone[0], bone[1], bone[2]);
        Matrix_RotYXZsuperpack(&mesh_rots, 0);

        if (extra_rotation != NULL
            && bone_flags & (BF_ROT_X | BF_ROT_Y | BF_ROT_Z)) {
            if (bone_flags & BF_ROT_Y) {
                Matrix_RotY(*extra_rotation++);
            }
            if (bone_flags & BF_ROT_X) {
                Matrix_RotX(*extra_rotation++);
            }
            if (bone_flags & BF_ROT_Z) {
                Matrix_RotZ(*extra_rotation++);
            }
        }

        bit <<= 1;
        if ((mesh_bits & bit) && (item->mesh_bits & bit)) {
            const int16_t fx_num = Effect_Create(item->room_num);
            if (fx_num != NO_ITEM) {
                FX *const fx = &g_Effects[fx_num];
                fx->pos.x = item->pos.x + (g_MatrixPtr->_03 >> W2V_SHIFT);
                fx->pos.y = item->pos.y + (g_MatrixPtr->_13 >> W2V_SHIFT);
                fx->pos.z = item->pos.z + (g_MatrixPtr->_23 >> W2V_SHIFT);
                fx->rot.y = (Random_GetControl() - 0x4000) * 2;
                fx->room_num = item->room_num;
                fx->speed = Random_GetControl() >> 8;
                fx->fall_speed = -Random_GetControl() >> 8;
                fx->counter = damage;
                fx->object_id = O_BODY_PART;
                fx->frame_num = object->mesh_idx + i;
                fx->shade = g_LsAdder - 0x300;
            }
            item->mesh_bits &= ~bit;
        }

        bone += 3;
    }

    Matrix_Pop();

    return !(item->mesh_bits & (INT32_MAX >> (31 - object->mesh_count)));
}

int16_t __cdecl Effect_MissileFlame(
    const int32_t x, const int32_t y, const int32_t z, int16_t speed,
    const int16_t y_rot, const int16_t room_num)
{
    const int16_t fx_num = Effect_Create(room_num);
    if (fx_num == NO_ITEM) {
        return fx_num;
    }

    FX *const fx = &g_Effects[fx_num];
    fx->pos.x = x;
    fx->pos.y = y;
    fx->pos.z = z;
    fx->rot.x = 0;
    fx->rot.y = y_rot;
    fx->rot.z = 0;
    fx->room_num = room_num;
    fx->speed = 200;
    fx->frame_num =
        ((g_Objects[O_MISSILE_FLAME].mesh_count + 1) * Random_GetDraw()) >> 15;
    fx->object_id = O_MISSILE_FLAME;
    fx->shade = 14 * 256;

    Missile_ShootAtLara(fx);

    if (g_Objects[O_DRAGON_FRONT].loaded) {
        fx->counter = 0x4000;
    } else {
        fx->counter = 20;
    }

    return fx_num;
}

void __cdecl Effect_CreateBartoliLight(const int16_t item_num)
{
    const ITEM *const item = &g_Items[item_num];

    const int16_t fx_num = Effect_Create(item->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->object_id = O_TWINKLE;

        fx->rot.y = 2 * Random_GetDraw();
        fx->pos.x = item->pos.x
            + ((BARTOLI_LIGHT_RANGE * Math_Sin(fx->rot.y)) >> W2V_SHIFT);
        fx->pos.z = item->pos.z
            + ((BARTOLI_LIGHT_RANGE * Math_Cos(fx->rot.y)) >> W2V_SHIFT);
        fx->pos.y = (Random_GetDraw() >> 2) + item->pos.y - WALL_L;
        fx->room_num = item->room_num;
        fx->counter = item_num;
        fx->frame_num = 0;
    }

    // clang-format off
    Output_AddDynamicLight(
        item->pos.x,
        item->pos.y,
        item->pos.z,
        ((4 * Random_GetDraw()) >> 15) + 12,
        ((4 * Random_GetDraw()) >> 15) + 10);
    // clang-format on
}

void __cdecl FX_AssaultStart(ITEM *const item)
{
    g_SaveGame.statistics.timer = 0;
    g_IsAssaultTimerActive = 1;
    g_IsAssaultTimerDisplay = 1;
    g_FlipEffect = -1;
    Stats_StartTimer();
}

void __cdecl CreateBubble(const XYZ_32 *const pos, const int16_t room_num)
{
    const int16_t fx_num = Effect_Create(room_num);
    if (fx_num == NO_ITEM) {
        return;
    }

    FX *const fx = &g_Effects[fx_num];
    fx->pos = *pos;
    fx->object_id = O_BUBBLE;
    fx->frame_num = -((Random_GetDraw() * 3) / 0x8000);
    fx->speed = 10 + ((Random_GetDraw() * 6) / 0x8000);
}

void __cdecl FX_Bubbles(ITEM *const item)
{
    const int32_t count = (Random_GetDraw() * 3) / 0x8000;
    if (count == 0) {
        return;
    }

    Sound_Effect(SFX_LARA_BUBBLES, &item->pos, SPM_UNDERWATER);

    XYZ_32 offset = {
        .x = 0,
        .y = 0,
        .z = 50,
    };
    Collide_GetJointAbsPosition(item, &offset, LM_HEAD);

    for (int32_t i = 0; i < count; i++) {
        CreateBubble(&offset, item->room_num);
    }
}

void __cdecl Splash(const ITEM *const item)
{
    const int32_t water_height = Room_GetWaterHeight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);
    int16_t room_num = item->room_num;
    Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);

    for (int32_t i = 0; i < 10; i++) {
        const int16_t fx_num = Effect_Create(room_num);
        if (fx_num == NO_ITEM) {
            continue;
        }

        FX *const fx = &g_Effects[fx_num];
        fx->object_id = O_SPLASH;
        fx->pos.x = item->pos.x;
        fx->pos.y = water_height;
        fx->pos.z = item->pos.z;
        fx->rot.y = 2 * Random_GetDraw() + PHD_180;
        fx->speed = Random_GetDraw() / 256;
        fx->frame_num = 0;
    }
}

void __cdecl FX_LaraHandsFree(ITEM *const item)
{
    g_Lara.gun_status = LGS_ARMLESS;
}

int16_t __cdecl Effect_GunShot(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t y_rot, const int16_t room_num)
{
    const int16_t fx_num = Effect_Create(room_num);
    if (fx_num == NO_ITEM) {
        return fx_num;
    }

    FX *const fx = &g_Effects[fx_num];
    fx->pos.x = x;
    fx->pos.y = y;
    fx->pos.z = z;
    fx->room_num = room_num;
    fx->rot.z = 0;
    fx->rot.x = 0;
    fx->rot.y = y_rot;
    fx->counter = 3;
    fx->frame_num = 0;
    fx->object_id = O_GUN_FLASH;
    fx->shade = HIGH_LIGHT;
    return fx_num;
}

int16_t __cdecl Effect_GunHit(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t y_rot, const int16_t room_num)
{
    XYZ_32 vec = { 0 };
    Collide_GetJointAbsPosition(
        g_LaraItem, &vec, Random_GetControl() * 25 / 0x7FFF);
    DoBloodSplat(
        vec.x, vec.y, vec.z, g_LaraItem->speed, g_LaraItem->rot.y,
        g_LaraItem->room_num);
    Sound_Effect(SFX_LARA_BULLETHIT, &g_LaraItem->pos, SPM_NORMAL);
    return Effect_GunShot(x, y, z, speed, y_rot, room_num);
}

int16_t __cdecl Effect_GunMiss(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t y_rot, const int16_t room_num)
{
    GAME_VECTOR pos = {
        .pos = {
            .x = g_LaraItem->pos.x + ((Random_GetDraw() - 0x4000) << 9) / 0x7FFF,
            .y = g_LaraItem->floor,
            .z = g_LaraItem->pos.z + ((Random_GetDraw() - 0x4000) << 9) / 0x7FFF,
        },
        .room_num = g_LaraItem->room_num,
    };
    Richochet(&pos);
    return Effect_GunShot(x, y, z, speed, y_rot, room_num);
}

int16_t __cdecl Knife(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t y_rot, const int16_t room_num)
{
    const int16_t fx_num = Effect_Create(room_num);
    if (fx_num == NO_ITEM) {
        return fx_num;
    }

    FX *const fx = &g_Effects[fx_num];
    fx->pos.x = x;
    fx->pos.y = y;
    fx->pos.z = z;
    fx->room_num = room_num;
    fx->rot.x = 0;
    fx->rot.y = y_rot;
    fx->rot.z = 0;
    fx->speed = 150;
    fx->frame_num = 0;
    fx->object_id = O_MISSILE_KNIFE;
    fx->shade = 3584;
    Missile_ShootAtLara(fx);
    return fx_num;
}

int16_t __cdecl DoBloodSplat(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t direction, const int16_t room_num)
{
    const int16_t fx_num = Effect_Create(room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->pos.x = x;
        fx->pos.y = y;
        fx->pos.z = z;
        fx->rot.y = direction;
        fx->speed = speed;
        fx->frame_num = 0;
        fx->object_id = O_BLOOD;
        fx->counter = 0;
    }
    return fx_num;
}

void __cdecl DoLotsOfBlood(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t direction, const int16_t room_num, const int32_t num)
{
    for (int32_t i = 0; i < num; i++) {
        DoBloodSplat(
            x - (Random_GetDraw() << 9) / 0x8000 + 256,
            y - (Random_GetDraw() << 9) / 0x8000 + 256,
            z - (Random_GetDraw() << 9) / 0x8000 + 256, speed, direction,
            room_num);
    }
}

void __cdecl Richochet(const GAME_VECTOR *const pos)
{
    const int16_t fx_num = Effect_Create(pos->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->object_id = O_RICOCHET;
        fx->pos = pos->pos;
        fx->counter = 4;
        fx->frame_num = -3 * Random_GetDraw() / 0x8000;
        Sound_Effect(SFX_LARA_RICOCHET, &fx->pos, SPM_NORMAL);
    }
}

void __cdecl FX_FinishLevel(ITEM *const item)
{
    g_LevelComplete = true;
}

void __cdecl FX_Turn180(ITEM *const item)
{
    item->rot.x = -item->rot.x;
    item->rot.y += PHD_180;
}

void __cdecl FX_FloorShake(ITEM *const item)
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

void __cdecl FX_LaraNormal(ITEM *const item)
{
    item->current_anim_state = LS_STOP;
    item->goal_anim_state = LS_STOP;
    item->anim_num = LA_STAND_STILL;
    item->frame_num = g_Anims[item->anim_num].frame_base;
    g_Camera.type = CAM_CHASE;
    Output_AlterFOV(GAME_FOV * PHD_DEGREE);
}

void __cdecl FX_Boiler(ITEM *const item)
{
    Sound_Effect(SFX_UNKNOWN_1, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void __cdecl FX_Flood(ITEM *const item)
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

void __cdecl FX_Rubble(ITEM *const item)
{
    Sound_Effect(SFX_MASSIVE_CRASH, NULL, SPM_NORMAL);
    g_Camera.bounce = -350;
    g_FlipEffect = -1;
}

void __cdecl FX_Chandelier(ITEM *const item)
{
    Sound_Effect(SFX_CHAIN_PULLEY, NULL, SPM_NORMAL);
    g_FlipTimer++;
    if (g_FlipTimer > FRAMES_PER_SECOND) {
        g_FlipEffect = -1;
    }
}

void __cdecl FX_Explosion(ITEM *const item)
{
    Sound_Effect(SFX_EXPLOSION_1, NULL, SPM_NORMAL);
    g_Camera.bounce = -75;
    g_FlipEffect = -1;
}

void __cdecl FX_Piston(ITEM *const item)
{
    Sound_Effect(SFX_PULLEY_CRANE, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void __cdecl FX_Curtain(ITEM *const item)
{
    Sound_Effect(SFX_CURTAIN, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void __cdecl FX_Statue(ITEM *const item)
{
    Sound_Effect(SFX_STONE_DOOR_SLIDE, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void __cdecl FX_SetChange(ITEM *const item)
{
    Sound_Effect(SFX_STAGE_BACKDROP, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}

void __cdecl FX_FlipMap(ITEM *const item)
{
    Room_FlipMap();
}

void __cdecl FX_LaraDrawRightGun(ITEM *const item)
{
    const OBJECT *const obj = Object_GetObject(O_LARA_PISTOLS);
    int16_t *tmp;
    SWAP(
        g_Lara.mesh_ptrs[LM_THIGH_R], g_Meshes[obj->mesh_idx + LM_THIGH_R],
        tmp);
    SWAP(g_Lara.mesh_ptrs[LM_HAND_R], g_Meshes[obj->mesh_idx + LM_HAND_R], tmp);
}

void __cdecl FX_LaraDrawLeftGun(ITEM *const item)
{
    const OBJECT *const obj = Object_GetObject(O_LARA_PISTOLS);
    int16_t *tmp;
    SWAP(
        g_Lara.mesh_ptrs[LM_THIGH_L], g_Meshes[obj->mesh_idx + LM_THIGH_L],
        tmp);
    SWAP(g_Lara.mesh_ptrs[LM_HAND_L], g_Meshes[obj->mesh_idx + LM_HAND_L], tmp);
}

void __cdecl FX_SwapMeshesWithMeshSwap1(ITEM *const item)
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

void __cdecl FX_SwapMeshesWithMeshSwap2(ITEM *const item)
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

void __cdecl FX_SwapMeshesWithMeshSwap3(ITEM *const item)
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

void __cdecl FX_InvisibilityOn(ITEM *const item)
{
    item->status = IS_INVISIBLE;
}

void __cdecl FX_InvisibilityOff(ITEM *const item)
{
    item->status = IS_ACTIVE;
}
