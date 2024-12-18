#include "game/objects/effects/missile_common.h"

#include "decomp/effects.h"
#include "game/effects.h"
#include "game/lara/control.h"
#include "game/lara/misc.h"
#include "game/math.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

void __cdecl Missile_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];

    if (fx->object_id == O_MISSILE_HARPOON
        && !(Room_Get(fx->room_num)->flags & RF_UNDERWATER)) {
        if (fx->rot.x > -0x3000) {
            fx->rot.x -= PHD_DEGREE;
        }
    }

    fx->pos.y += (fx->speed * Math_Sin(-fx->rot.x)) >> W2V_SHIFT;
    const int32_t speed = (fx->speed * Math_Cos(fx->rot.x)) >> W2V_SHIFT;
    fx->pos.z += (speed * Math_Cos(fx->rot.y)) >> W2V_SHIFT;
    fx->pos.x += (speed * Math_Sin(fx->rot.y)) >> W2V_SHIFT;

    int16_t room_num = fx->room_num;
    const SECTOR *const sector =
        Room_GetSector(fx->pos.x, fx->pos.y, fx->pos.z, &room_num);

    const int32_t height =
        Room_GetHeight(sector, fx->pos.x, fx->pos.y, fx->pos.z);
    const int32_t ceiling =
        Room_GetCeiling(sector, fx->pos.x, fx->pos.y, fx->pos.z);
    if (fx->pos.y >= height || fx->pos.y <= ceiling) {
        if (fx->object_id == O_MISSILE_KNIFE
            || fx->object_id == O_MISSILE_HARPOON) {
            fx->object_id = O_RICOCHET;
            fx->frame_num = Random_GetControl() / -11000;
            fx->speed = 0;
            fx->counter = 6;
            Sound_Effect(SFX_CIRCLE_BLADE_HIT, &fx->pos, SPM_NORMAL);
        } else if (fx->object_id == O_MISSILE_FLAME) {
            Output_AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 14, 11);
            Effect_Kill(fx_num);
        }
        return;
    }

    if (room_num != fx->room_num) {
        Effect_NewRoom(fx_num, room_num);
    }

    if (fx->object_id == O_MISSILE_FLAME) {
        if (Lara_IsNearItem(&fx->pos, 350)) {
            Lara_TakeDamage(3, true);
            Lara_CatchFire();
            return;
        }
    } else if (Lara_IsNearItem(&fx->pos, 200)) {
        if (fx->object_id == O_MISSILE_KNIFE
            || fx->object_id == O_MISSILE_HARPOON) {
            g_LaraItem->hit_points -= 50;
            fx->object_id = O_BLOOD;
            Sound_Effect(SFX_CRUNCH_1, &fx->pos, SPM_NORMAL);
        }
        g_LaraItem->hit_status = 1;

        fx->rot.y = g_LaraItem->rot.y;
        fx->frame_num = 0;
        fx->speed = g_LaraItem->speed;
        fx->counter = 0;
    }

    if (fx->object_id == O_MISSILE_HARPOON
        && (Room_Get(fx->room_num)->flags & RF_UNDERWATER)) {
        CreateBubble(&fx->pos, fx->room_num);
    } else if (fx->object_id == O_MISSILE_FLAME) {
        if (!fx->counter--) {
            Output_AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 14, 11);
            Sound_Effect(SFX_DRAGON_FIRE, &fx->pos, SPM_NORMAL);
            Effect_Kill(fx_num);
        }
    } else if (fx->object_id == O_MISSILE_KNIFE) {
        fx->rot.z += 30 * PHD_DEGREE;
    }
}

void __cdecl Missile_ShootAtLara(FX *const fx)
{
    const int32_t dx = g_LaraItem->pos.x - fx->pos.x;
    const int32_t dy = g_LaraItem->pos.y - fx->pos.y;
    const int32_t dz = g_LaraItem->pos.z - fx->pos.z;

    const BOUNDS_16 *const bounds = Item_GetBoundsAccurate(g_LaraItem);
    const int32_t dist_vert =
        dy + bounds->max_y + 3 * (bounds->min_y - bounds->max_y) / 4;
    const int32_t dist_horz = Math_Sqrt(SQUARE(dz) + SQUARE(dx));
    fx->rot.x = -Math_Atan(dist_horz, dist_vert);
    fx->rot.y = Math_Atan(dz, dx);
    fx->rot.x += (Random_GetControl() - 0x4000) / 64;
    fx->rot.y += (Random_GetControl() - 0x4000) / 64;
}
