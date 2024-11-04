#include "game/objects/effects/ricochet.h"

#include "global/funcs.h"

void Ricochet_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_RICOCHET);
    obj->control = Richochet_Control;
}
