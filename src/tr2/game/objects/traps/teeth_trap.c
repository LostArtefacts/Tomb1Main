#include "game/objects/traps/teeth_trap.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void TeethTrap_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_TEETH_TRAP);
    obj->control = TeethTrap_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
