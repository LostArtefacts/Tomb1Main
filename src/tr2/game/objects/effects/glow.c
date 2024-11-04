#include "game/objects/effects/glow.h"

#include "global/funcs.h"

void Glow_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GLOW);
    obj->control = Glow_Control;
}
