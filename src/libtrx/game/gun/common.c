#include "config.h"
#include "game/const.h"
#include "game/gun.h"
#include "game/lara/common.h"
#include "game/output.h"

void Gun_AddDynamicLight(void)
{
    if (!g_Config.visuals.enable_gun_lighting) {
        return;
    }

    const ITEM *const lara_item = Lara_GetItem();
    const int32_t c = Math_Cos(lara_item->rot.y);
    const int32_t s = Math_Sin(lara_item->rot.y);
    const XYZ_32 pos = {
        .x = lara_item->pos.x + (s >> (W2V_SHIFT - 10)),
        .y = lara_item->pos.y - WALL_L / 2,
        .z = lara_item->pos.z + (c >> (W2V_SHIFT - 10)),
    };
    Output_AddDynamicLight(pos, 12, 11);
}
