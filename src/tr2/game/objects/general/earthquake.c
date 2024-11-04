#include "game/objects/general/earthquake.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Earthquake_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_EARTHQUAKE);
    obj->control = Earthquake_Control;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
