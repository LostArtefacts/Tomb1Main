#include "game/objects/general/alarm_sound.h"

#include "global/funcs.h"

void AlarmSound_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_ALARM_SOUND);
    obj->control = AlarmSound_Control;
    obj->save_flags = 1;
}
