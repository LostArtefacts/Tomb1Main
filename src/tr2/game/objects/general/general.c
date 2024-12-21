#include "game/objects/general/general.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void General_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GENERAL);
    obj->control = GeneralControl;
    obj->collision = Object_Collision;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->water_creature = 1;
}
