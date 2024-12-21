#include "game/objects/traps/ember_emitter.h"

#include "game/effects.h"
#include "game/objects/common.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/vars.h"

void __cdecl EmberEmitter_Control(const int16_t item_num)
{
    const ITEM *const item = Item_Get(item_num);
    const int16_t fx_num = Effect_Create(item->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->pos.x = item->pos.x;
        fx->pos.y = item->pos.y;
        fx->pos.z = item->pos.z;
        fx->rot.y = 2 * Random_GetControl() + 0x8000;
        fx->speed = Random_GetControl() >> 10;
        fx->fall_speed = Random_GetControl() / -200;
        fx->frame_num = (-4 * Random_GetControl()) / 0x7FFF;
        fx->object_id = O_EMBER;
        Sound_Effect(SFX_SANDBAG_HIT, &item->pos, SPM_NORMAL);
    }
}

void EmberEmitter_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_EMBER_EMITTER);
    obj->control = EmberEmitter_Control;
    obj->collision = Object_Collision;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
