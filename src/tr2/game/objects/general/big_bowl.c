#include "game/objects/general/big_bowl.h"

#include "global/funcs.h"

void BigBowl_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BIG_BOWL);
    obj->control = BigBowl_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
