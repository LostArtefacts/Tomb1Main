#include "game/objects/general/lara_alarm.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void LaraAlarm_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_LARA_ALARM);
    obj->control = LaraAlarm_Control;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
