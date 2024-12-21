#include "game/objects/traps/propeller.h"

#include "global/funcs.h"

void Propeller_Setup(OBJECT *const obj, const bool is_underwater)
{
    obj->control = Propeller_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->water_creature = is_underwater;
}
