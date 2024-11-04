#include "game/objects/effects/waterfall.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Waterfall_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WATERFALL);
    obj->control = Waterfall_Control;
    obj->draw_routine = Object_DrawDummyItem;
}
