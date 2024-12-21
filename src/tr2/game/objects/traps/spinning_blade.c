#include "game/objects/traps/spinning_blade.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void SpinningBlade_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SPINNING_BLADE);
    obj->initialise = KillerStatue_Initialise;
    obj->control = SpinningBlade_Control;
    obj->collision = Object_Collision;
    obj->save_position = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
