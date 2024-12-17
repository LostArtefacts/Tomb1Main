#include "decomp/flares.h"

#include "config.h"
#include "decomp/effects.h"
#include "game/gun/gun.h"
#include "game/input.h"
#include "game/inventory/backpack.h"
#include "game/lara/misc.h"
#include "game/math.h"
#include "game/matrix.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#define FLARE_INTENSITY 12
#define FLARE_FALL_OFF 11
#define MAX_FLARE_AGE (60 * FRAMES_PER_SECOND) // = 1800
#define FLARE_OLD_AGE (MAX_FLARE_AGE - 2 * FRAMES_PER_SECOND) // = 1740
#define FLARE_YOUNG_AGE (FRAMES_PER_SECOND) // = 30

static bool M_CanThrowFlare(void);

static bool M_CanThrowFlare(void)
{
    if (g_Lara.gun_status != LGS_ARMLESS) {
        return false;
    }

    return !g_Config.gameplay.fix_flare_throw_priority
        || (!g_LaraItem->gravity && !g_Input.jump)
        || g_LaraItem->current_anim_state == LS_FAST_FALL
        || g_LaraItem->current_anim_state == LS_SWAN_DIVE
        || g_LaraItem->current_anim_state == LS_FAST_DIVE;
}

int32_t __cdecl Flare_DoLight(const XYZ_32 *const pos, const int32_t flare_age)
{
    if (flare_age >= MAX_FLARE_AGE) {
        return false;
    }

    const int32_t random = Random_GetDraw();
    const int32_t x = pos->x + (random & 0xF);
    const int32_t y = pos->y;
    const int32_t z = pos->z;

    if (flare_age < FLARE_YOUNG_AGE) {
        const int32_t intensity = FLARE_INTENSITY
                * (flare_age - FLARE_YOUNG_AGE) / (2 * FLARE_YOUNG_AGE)
            + FLARE_INTENSITY;
        AddDynamicLight(x, y, z, intensity, FLARE_FALL_OFF);
        return true;
    }

    if (flare_age < FLARE_OLD_AGE) {
        AddDynamicLight(x, y, z, FLARE_INTENSITY, FLARE_FALL_OFF);
        return true;
    }

    if (random > 0x2000) {
        AddDynamicLight(
            x, y, z, FLARE_INTENSITY - (random & 3), FLARE_FALL_OFF);
        return true;
    }

    AddDynamicLight(x, y, z, FLARE_INTENSITY, FLARE_FALL_OFF / 2);
    return false;
}

void __cdecl Flare_DoInHand(const int32_t flare_age)
{
    XYZ_32 vec = {
        .x = 11,
        .y = 32,
        .z = 41,
    };
    Lara_GetJointAbsPosition(&vec, LM_HAND_L);

    g_Lara.left_arm.flash_gun = Flare_DoLight(&vec, flare_age);

    if (g_Lara.flare_age < MAX_FLARE_AGE) {
        g_Lara.flare_age++;
        if (g_Rooms[g_LaraItem->room_num].flags & RF_UNDERWATER) {
            Sound_Effect(SFX_LARA_FLARE_BURN, &g_LaraItem->pos, SPM_UNDERWATER);
            if (Random_GetDraw() < 0x4000) {
                CreateBubble(&vec, g_LaraItem->room_num);
            }
        } else {
            Sound_Effect(SFX_LARA_FLARE_BURN, &g_LaraItem->pos, SPM_NORMAL);
        }
    } else if (M_CanThrowFlare()) {
        g_Lara.gun_status = LGS_UNDRAW;
    }
}

void __cdecl Flare_DrawInAir(const ITEM *const item)
{
    int32_t rate;
    FRAME_INFO *frames[2];
    Item_GetFrames(item, frames, &rate);
    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);
    const int32_t clip = S_GetObjectBounds(&frames[0]->bounds);
    if (clip != 0) {
        Output_CalculateObjectLighting(item, &frames[0]->bounds);
        Output_InsertPolygons(g_Meshes[g_Objects[O_FLARE_ITEM].mesh_idx], clip);
        if ((int32_t)item->data & 0x8000) {
            Matrix_TranslateRel(-6, 6, 80);
            Matrix_RotX(-90 * PHD_DEGREE);
            Matrix_RotY((int16_t)(2 * Random_GetDraw()));
            S_CalculateStaticLight(8 * 256);
            Output_InsertPolygons(
                g_Meshes[g_Objects[O_FLARE_FIRE].mesh_idx], clip);
        }
    }
    Matrix_Pop();
}

void __cdecl Flare_Create(const bool thrown)
{
    const int16_t item_num = Item_Create();
    if (item_num == NO_ITEM) {
        return;
    }

    ITEM *const item = Item_Get(item_num);
    item->object_id = O_FLARE_ITEM;
    item->room_num = g_LaraItem->room_num;

    XYZ_32 vec = {
        .x = -16,
        .y = 32,
        .z = 42,
    };
    Lara_GetJointAbsPosition(&vec, LM_HAND_L);

    const SECTOR *const sector =
        Room_GetSector(vec.x, vec.y, vec.z, &item->room_num);
    const int32_t height = Room_GetHeight(sector, vec.x, vec.y, vec.z);
    if (height < vec.y) {
        item->pos.x = g_LaraItem->pos.x;
        item->pos.y = vec.y;
        item->pos.z = g_LaraItem->pos.z;
        item->rot.y = -g_LaraItem->rot.y;
        item->room_num = g_LaraItem->room_num;
    } else {
        item->pos.x = vec.x;
        item->pos.y = vec.y;
        item->pos.z = vec.z;
        if (thrown) {
            item->rot.y = g_LaraItem->rot.y;
        } else {
            item->rot.y = g_LaraItem->rot.y - PHD_45;
        }
    }

    Item_Initialise(item_num);

    item->rot.z = 0;
    item->rot.x = 0;
    item->shade_1 = -1;

    if (thrown) {
        item->speed = g_LaraItem->speed + 50;
        item->fall_speed = g_LaraItem->fall_speed - 50;
    } else {
        item->speed = g_LaraItem->speed + 10;
        item->fall_speed = g_LaraItem->fall_speed + 50;
    }

    if (Flare_DoLight(&item->pos, g_Lara.flare_age)) {
        item->data = (void *)(g_Lara.flare_age | 0x8000);
    } else {
        item->data = (void *)(g_Lara.flare_age & ~0x8000);
    }

    Item_AddActive(item_num);
    item->status = IS_ACTIVE;
}

void __cdecl Flare_SetArm(const int32_t frame)
{
    int16_t anim_base;

    if (frame < LF_FL_THROW) {
        anim_base = g_Objects[O_LARA_FLARE].anim_idx;
    } else if (frame < LF_FL_DRAW) {
        anim_base = g_Objects[O_LARA_FLARE].anim_idx + 1;
    } else if (frame < LF_FL_IGNITE) {
        anim_base = g_Objects[O_LARA_FLARE].anim_idx + 2;
    } else if (frame < LF_FL_2_HOLD) {
        anim_base = g_Objects[O_LARA_FLARE].anim_idx + 3;
    } else {
        anim_base = g_Objects[O_LARA_FLARE].anim_idx + 4;
    }

    g_Lara.left_arm.anim_num = anim_base;
    g_Lara.left_arm.frame_base = g_Anims[g_Lara.left_arm.anim_num].frame_ptr;
}

void __cdecl Flare_Draw(void)
{
    if (g_LaraItem->current_anim_state == LS_FLARE_PICKUP
        || g_LaraItem->current_anim_state == LS_PICKUP) {
        Flare_DoInHand(g_Lara.flare_age);
        g_Lara.flare_control_left = 0;
        g_Lara.left_arm.frame_num = LF_FL_2_HOLD - 2;
        Flare_SetArm(g_Lara.left_arm.frame_num);
        return;
    }

    int32_t frame_num = g_Lara.left_arm.frame_num + 1;

    g_Lara.flare_control_left = 1;

    if (frame_num < LF_FL_DRAW || frame_num > LF_FL_2_HOLD - 1) {
        frame_num = LF_FL_DRAW;
    } else if (frame_num == LF_FL_DRAW_GOT_IT) {
        Flare_DrawMeshes();
        if (!g_SaveGame.bonus_flag) {
            Inv_RemoveItem(O_FLARES_ITEM);
        }
    } else if (frame_num >= LF_FL_IGNITE && frame_num <= LF_FL_2_HOLD - 2) {
        if (frame_num == LF_FL_IGNITE) {
            if ((g_Rooms[g_LaraItem->room_num].flags & RF_UNDERWATER) != 0) {
                Sound_Effect(
                    SFX_LARA_FLARE_IGNITE, &g_LaraItem->pos, SPM_UNDERWATER);
            } else {
                Sound_Effect(
                    SFX_LARA_FLARE_IGNITE, &g_LaraItem->pos, SPM_NORMAL);
            }
            g_Lara.flare_age = 0;
        }
        Flare_DoInHand(g_Lara.flare_age);
    } else if (frame_num == LF_FL_2_HOLD - 1) {
        Flare_Ready();
        Flare_DoInHand(g_Lara.flare_age);
        frame_num = LF_FL_HOLD;
    }

    g_Lara.left_arm.frame_num = frame_num;
    Flare_SetArm(frame_num);
}

void __cdecl Flare_Undraw(void)
{
    int16_t frame_num_1 = g_Lara.left_arm.frame_num;
    int16_t frame_num_2 = g_Lara.flare_frame;

    g_Lara.flare_control_left = 1;

    if (g_LaraItem->goal_anim_state == LS_STOP && g_Lara.skidoo == NO_ITEM) {
        if (g_LaraItem->anim_num == LA_STAND_IDLE) {
            frame_num_2 = g_Anims[LA_FLARE_THROW].frame_base + frame_num_1;
            g_LaraItem->anim_num = LA_FLARE_THROW;
            g_Lara.flare_frame = frame_num_2;
            g_LaraItem->frame_num = frame_num_2;
        }

        if (g_LaraItem->anim_num == LA_FLARE_THROW) {
            g_Lara.flare_control_left = 0;

            if (frame_num_2
                >= g_Anims[LA_FLARE_THROW].frame_base + LF_FL_THROW_FT - 1) {
                g_Lara.gun_type = g_Lara.last_gun_type;
                g_Lara.request_gun_type = g_Lara.last_gun_type;
                g_Lara.gun_status = LGS_ARMLESS;
                Gun_InitialiseNewWeapon();
                g_Lara.target = NULL;
                g_Lara.right_arm.lock = 0;
                g_Lara.left_arm.lock = 0;
                g_LaraItem->anim_num = LA_STAND_STILL;
                g_Lara.flare_frame = g_Anims[g_LaraItem->anim_num].frame_base;
                g_LaraItem->frame_num = g_Lara.flare_frame;
                g_LaraItem->current_anim_state = LS_STOP;
                g_LaraItem->goal_anim_state = LS_STOP;
                return;
            }
            g_Lara.flare_frame = frame_num_2 + 1;
        }
    } else if (
        g_LaraItem->current_anim_state == LS_STOP && g_Lara.skidoo == -1) {
        g_LaraItem->anim_num = LA_STAND_STILL;
        g_LaraItem->frame_num = g_Anims[g_LaraItem->anim_num].frame_base;
    }

    if (frame_num_1 == LF_FL_HOLD) {
        frame_num_1 = LF_FL_THROW;
    } else if (frame_num_1 >= LF_FL_IGNITE && frame_num_1 < LF_FL_2_HOLD) {
        frame_num_1++;
        if (frame_num_1 == LF_FL_2_HOLD - 1) {
            frame_num_1 = LF_FL_THROW;
        }
    } else if (frame_num_1 >= LF_FL_THROW && frame_num_1 < LF_FL_DRAW) {
        frame_num_1++;
        if (frame_num_1 == LF_FL_THROW_RELEASE) {
            Flare_Create(true);
            Flare_UndrawMeshes();
        } else if (frame_num_1 == LF_FL_DRAW) {
            frame_num_1 = 0;
            g_Lara.gun_type = g_Lara.last_gun_type;
            g_Lara.request_gun_type = g_Lara.last_gun_type;
            g_Lara.gun_status = LGS_ARMLESS;
            Gun_InitialiseNewWeapon();
            g_Lara.target = NULL;
            g_Lara.flare_control_left = 0;
            g_Lara.right_arm.lock = 0;
            g_Lara.left_arm.lock = 0;
            g_Lara.flare_frame = 0;
        }
    } else if (frame_num_1 >= LF_FL_2_HOLD && frame_num_1 < LF_FL_END) {
        frame_num_1++;
        if (frame_num_1 == LF_FL_END) {
            frame_num_1 = LF_FL_THROW;
        }
    }

    if (frame_num_1 >= LF_FL_THROW && frame_num_1 < LF_FL_THROW_RELEASE) {
        Flare_DoInHand(g_Lara.flare_age);
    }

    g_Lara.left_arm.frame_num = frame_num_1;
    Flare_SetArm(frame_num_1);
}

void __cdecl Flare_DrawMeshes(void)
{
    g_Lara.mesh_ptrs[LM_HAND_L] =
        g_Meshes[g_Objects[O_LARA_FLARE].mesh_idx + LM_HAND_L];
}

void __cdecl Flare_UndrawMeshes(void)
{
    g_Lara.mesh_ptrs[LM_HAND_L] =
        g_Meshes[g_Objects[O_LARA].mesh_idx + LM_HAND_L];
}

void __cdecl Flare_Ready(void)
{
    g_Lara.gun_status = LGS_ARMLESS;
    g_Lara.left_arm.rot.x = 0;
    g_Lara.left_arm.rot.y = 0;
    g_Lara.left_arm.rot.z = 0;
    g_Lara.right_arm.rot.x = 0;
    g_Lara.right_arm.rot.y = 0;
    g_Lara.right_arm.rot.z = 0;
    g_Lara.left_arm.lock = 0;
    g_Lara.right_arm.lock = 0;
    g_Lara.target = NULL;
}

void __cdecl Flare_Control(const int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    if (item->fall_speed) {
        item->rot.x += PHD_DEGREE * 3;
        item->rot.z += PHD_DEGREE * 5;
    } else {
        item->rot.x = 0;
        item->rot.z = 0;
    }

    const int32_t x = item->pos.x;
    const int32_t y = item->pos.y;
    const int32_t z = item->pos.z;
    item->pos.z += (item->speed * Math_Cos(item->rot.y)) >> W2V_SHIFT;
    item->pos.x += (item->speed * Math_Sin(item->rot.y)) >> W2V_SHIFT;

    if (g_Rooms[item->room_num].flags & RF_UNDERWATER) {
        item->fall_speed += (5 - item->fall_speed) / 2;
        item->speed = item->speed + (5 - item->speed) / 2;
    } else {
        item->fall_speed += GRAVITY;
    }
    item->pos.y += item->fall_speed;

    int16_t room_num = item->room_num;
    const SECTOR *const sector =
        Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);

    const int32_t height =
        Room_GetHeight(sector, item->pos.x, item->pos.y, item->pos.z);
    if (item->pos.y < height) {
        const int32_t ceiling =
            Room_GetCeiling(sector, item->pos.x, item->pos.y, item->pos.z);
        if (item->pos.y < ceiling) {
            item->fall_speed = -item->fall_speed;
            item->pos.y = ceiling;
        }
    } else {
        if (y > height) {
            item->pos.x = x;
            item->pos.y = y;
            item->pos.z = z;
            item->rot.y += PHD_180;
            item->speed /= 2;
            room_num = item->room_num;
        } else {
            if (item->fall_speed > 40) {
                item->fall_speed = 40 - item->fall_speed;
                CLAMPL(item->fall_speed, -100);
            } else {
                item->fall_speed = 0;
                item->speed -= 3;
                CLAMPL(item->speed, 0);
            }
            item->pos.y = height;
        }
    }

    if (room_num != item->room_num) {
        Item_NewRoom(item_num, room_num);
    }

    int32_t flare_age = (int32_t)item->data & 0x7FFF;
    if (flare_age < MAX_FLARE_AGE) {
        flare_age++;
    } else if (item->fall_speed == 0 && item->speed == 0) {
        Item_Kill(item_num);
        return;
    }

    if (Flare_DoLight(&item->pos, flare_age)) {
        flare_age |= 0x8000u;
        if ((g_Rooms[item->room_num].flags & RF_UNDERWATER)) {
            Sound_Effect(SFX_LARA_FLARE_BURN, &item->pos, SPM_UNDERWATER);
            if (Random_GetDraw() < 0x4000) {
                CreateBubble(&item->pos, item->room_num);
            }
        } else {
            Sound_Effect(SFX_LARA_FLARE_BURN, &item->pos, SPM_NORMAL);
        }
    }

    item->data = (void *)flare_age;
}
