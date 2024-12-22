#include "game/objects/general/lara_alarm.h"

#include "game/objects/common.h"
#include "game/sound.h"

void __cdecl LaraAlarm_Control(const int16_t item_num)
{
    const ITEM *const item = Item_Get(item_num);
    if ((item->flags & IF_CODE_BITS) == IF_CODE_BITS) {
        Sound_Effect(SFX_BURGLAR_ALARM, &item->pos, SPM_NORMAL);
    }
}

void LaraAlarm_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_LARA_ALARM);
    obj->control = LaraAlarm_Control;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
