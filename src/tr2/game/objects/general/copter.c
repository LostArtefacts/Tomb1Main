#include "game/objects/general/copter.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Copter_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_COPTER);
    obj->control = CopterControl;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
