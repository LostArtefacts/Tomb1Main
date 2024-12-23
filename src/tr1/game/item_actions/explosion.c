#include "game/item_actions/explosion.h"

#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#include <stddef.h>

void ItemAction_Explosion(ITEM *item)
{
    Sound_Effect(SFX_EXPLOSION_FX, NULL, SPM_NORMAL);
    g_Camera.bounce = -75;
    g_FlipEffect = -1;
}
