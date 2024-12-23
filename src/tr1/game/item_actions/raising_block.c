#include "game/item_actions/raising_block.h"

#include "game/room.h"
#include "game/sound.h"

#include <stddef.h>

void ItemAction_RaisingBlock(ITEM *item)
{
    Sound_Effect(SFX_RAISINGBLOCK_FX, NULL, SPM_NORMAL);
    g_FlipEffect = -1;
}
