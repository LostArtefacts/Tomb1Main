#include "game/objects/effects/gun_flash.h"

#include "global/funcs.h"

void GunFlash_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GUN_FLASH);
    obj->control = GunFlash_Control;
}
