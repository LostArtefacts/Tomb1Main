#include "game/objects/general/ding_dong.h"

#include "game/objects/common.h"
#include "game/sound.h"

void __cdecl DingDong_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if ((item->flags & IF_CODE_BITS) == IF_CODE_BITS) {
        Sound_Effect(SFX_DOORBELL, &item->pos, SPM_NORMAL);
        item->flags -= IF_CODE_BITS;
    }
}

void DingDong_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DING_DONG);
    obj->control = DingDong_Control;
    obj->draw_routine = Object_DrawDummyItem;
}
