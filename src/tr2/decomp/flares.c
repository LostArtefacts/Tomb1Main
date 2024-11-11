#include "decomp/flares.h"

#include "decomp/effects.h"
#include "game/lara/misc.h"
#include "game/math.h"
#include "game/matrix.h"
#include "game/output.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/funcs.h"
#include "global/vars.h"

#define FLARE_INTENSITY 12
#define FLARE_FALL_OFF 11
#define MAX_FLARE_AGE (60 * FRAMES_PER_SECOND) // = 1800
#define FLARE_OLD_AGE (MAX_FLARE_AGE - 2 * FRAMES_PER_SECOND) // = 1740
#define FLARE_YOUNG_AGE (FRAMES_PER_SECOND) // = 30

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
    } else if (g_Lara.gun_status == LGS_ARMLESS) {
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
        Output_CalculateObjectLighting(item, frames[0]);
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
