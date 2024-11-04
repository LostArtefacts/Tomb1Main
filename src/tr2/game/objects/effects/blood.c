#include "game/objects/effects/blood.h"

#include "global/funcs.h"

void Blood_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BLOOD);
    obj->control = Blood_Control;
    obj->semi_transparent = 1;
}
