#include "game/item_actions/stairs2slope.h"

#include "game/room.h"
#include "game/sound.h"

#include <stddef.h>

void ItemAction_Stairs2Slope(ITEM *item)
{
    if (g_FlipTimer == 5) {
        Sound_Effect(SFX_STAIRS_2_SLOPE_FX, NULL, SPM_NORMAL);
        g_FlipEffect = -1;
    }
    g_FlipTimer++;
}
