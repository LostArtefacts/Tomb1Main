#include "game/objects/general/secret.h"

#include "game/objects/common.h"
#include "game/objects/general/pickup.h"
#include "global/funcs.h"

void Secret2_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SECRET_2);
    Pickup_Setup(obj);
    // TODO: why is it so special?
    obj->control = Secret_Control;
}
