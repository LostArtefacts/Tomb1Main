#include "game/objects/effects/explosion.h"

#include "global/funcs.h"

void Explosion_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_EXPLOSION);
    obj->control = Explosion_Control;
    obj->semi_transparent = 1;
}
