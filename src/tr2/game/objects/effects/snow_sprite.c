#include "game/objects/effects/snow_sprite.h"

#include "global/funcs.h"

void SnowSprite_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SNOW_SPRITE);
    obj->control = SnowSprite_Control;
}
