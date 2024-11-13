#include "game/objects/effects/missile_harpoon.h"

#include "game/objects/common.h"
#include "game/objects/effects/missile_common.h"

void MissileHarpoon_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_MISSILE_HARPOON);
    obj->control = Missile_Control;
    obj->save_position = 1;
}
