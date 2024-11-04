#include "game/objects/effects/missile_flame.h"

#include "global/funcs.h"

void MissileFlame_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_MISSILE_FLAME);
    obj->control = Missile_Control;
    obj->semi_transparent = 1;
}
