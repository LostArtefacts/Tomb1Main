#include "game/objects/effects/bubbles.h"

#include "global/funcs.h"

void Bubbles_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BUBBLES);
    obj->control = Bubbles_Control;
}
