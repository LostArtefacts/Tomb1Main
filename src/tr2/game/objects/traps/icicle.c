#include "game/objects/traps/icicle.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Icicle_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_ICICLE);
    obj->control = Icicle_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_position = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
