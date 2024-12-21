#include "game/objects/traps/pendulum.h"

#include "global/funcs.h"

void Pendulum_Setup(OBJECT *const obj)
{
    obj->control = Pendulum_Control;
    obj->collision = Object_Collision;
    obj->shadow_size = 128;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
