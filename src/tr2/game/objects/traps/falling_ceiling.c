#include "game/objects/traps/falling_ceiling.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void FallingCeiling_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_FALLING_CEILING);
    obj->control = FallingCeiling_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_position = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
