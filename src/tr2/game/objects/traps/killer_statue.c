#include "game/objects/traps/killer_statue.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void KillerStatue_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_KILLER_STATUE);
    obj->initialise = KillerStatue_Initialise;
    obj->control = KillerStatue_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
