#include "game/objects/traps/flame_emitter.h"

#include "game/effects.h"
#include "game/items.h"
#include "game/objects/common.h"
#include "global/vars.h"

void __cdecl FlameEmitter_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    if (!Item_IsTriggerActive(item)) {
        if (item->data != NULL) {
            const int32_t flame_num = ((int32_t)(intptr_t)item->data) - 1;
            Effect_Kill(flame_num);
            item->data = NULL;
        }
    } else if (item->data == NULL) {
        const int16_t fx_num = Effect_Create(item->room_num);
        if (fx_num != NO_ITEM) {
            FX *const fx = &g_Effects[fx_num];
            fx->pos.x = item->pos.x;
            fx->pos.y = item->pos.y;
            fx->pos.z = item->pos.z;
            fx->frame_num = 0;
            fx->object_id = O_FLAME;
            fx->counter = 0;
        }
        item->data = (void *)(fx_num + 1);
    }
}

void FlameEmitter_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_FLAME_EMITTER);
    obj->control = FlameEmitter_Control;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
