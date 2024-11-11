#include "decomp/flares.h"

#include "decomp/effects.h"
#include "game/lara/misc.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/funcs.h"
#include "global/vars.h"

#define MAX_FLARE_AGE (60 * FRAMES_PER_SECOND)

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
