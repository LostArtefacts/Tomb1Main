#include "game/objects/general/bell.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Bell_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BELL);
    obj->control = Bell_Control;
    obj->collision = Object_Collision;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
