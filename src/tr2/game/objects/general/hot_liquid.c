#include "game/objects/general/hot_liquid.h"

#include "global/funcs.h"

void HotLiquid_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_HOT_LIQUID);
    obj->control = HotLiquid_Control;
    obj->semi_transparent = 1;
}
