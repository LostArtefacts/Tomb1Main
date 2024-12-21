#include "game/objects/traps/blade.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Blade_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BLADE);
    obj->initialise = Blade_Initialise;
    obj->control = Blade_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
