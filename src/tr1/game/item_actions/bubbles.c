#include "game/item_actions/bubbles.h"

#include "game/collide.h"
#include "game/effects.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/vars.h"

#include <stdint.h>

void ItemAction_Bubbles(ITEM *item)
{
    // XXX: until we get Robolara, it makes sense for her to breathe underwater
    if (g_Lara.water_status == LWS_CHEAT
        && !(g_RoomInfo[g_LaraItem->room_num].flags & RF_UNDERWATER)) {
        return;
    }

    int32_t count = (Random_GetDraw() * 3) / 0x8000;
    if (!count) {
        return;
    }

    Sound_Effect(SFX_LARA_BUBBLES, &item->pos, SPM_UNDERWATER);

    XYZ_32 offset = {
        .x = 0,
        .y = 0,
        .z = 50,
    };
    Collide_GetJointAbsPosition(item, &offset, LM_HEAD);

    for (int i = 0; i < count; i++) {
        int16_t effect_num = Effect_Create(item->room_num);
        if (effect_num != NO_EFFECT) {
            EFFECT *effect = Effect_Get(effect_num);
            effect->pos.x = offset.x;
            effect->pos.y = offset.y;
            effect->pos.z = offset.z;
            effect->object_id = O_BUBBLES_1;
            effect->frame_num = -((Random_GetDraw() * 3) / 0x8000);
            effect->speed = 10 + ((Random_GetDraw() * 6) / 0x8000);
        }
    }
}
