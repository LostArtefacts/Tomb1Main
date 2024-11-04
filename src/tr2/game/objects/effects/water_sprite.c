#include "game/objects/effects/water_sprite.h"

#include "global/funcs.h"

void WaterSprite_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WATER_SPRITE);
    obj->control = WaterSprite_Control;
    obj->semi_transparent = 1;
}
