#include "game/objects/common.h"
#include "game/objects/general/copter.h"
#include "global/funcs.h"

void MiniCopter_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_MINI_COPTER);
    obj->control = MiniCopterControl;
    obj->save_position = 1;
    obj->save_flags = 1;
}
