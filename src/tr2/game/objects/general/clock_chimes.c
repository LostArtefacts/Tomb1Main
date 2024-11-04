#include "game/objects/general/clock_chimes.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void ClockChimes_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_CLOCK_CHIMES);
    obj->control = ClockChimes_Control;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
