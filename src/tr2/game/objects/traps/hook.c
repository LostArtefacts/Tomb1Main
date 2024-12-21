#include "game/objects/traps/hook.h"

#include "game/creature.h"
#include "game/objects/common.h"
#include "global/funcs.h"

void Hook_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_HOOK);
    obj->control = Hook_Control;
    obj->collision = Creature_Collision;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
