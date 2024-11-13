#include "game/objects/effects/missile_knife.h"

#include "game/objects/common.h"
#include "game/objects/effects/missile_common.h"

void MissileKnife_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_MISSILE_KNIFE);
    obj->control = Missile_Control;
    obj->save_position = 1;
}
