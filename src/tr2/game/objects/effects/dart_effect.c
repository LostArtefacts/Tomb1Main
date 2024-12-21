#include "game/objects/effects/dart_effect.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void DartEffect_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DART_EFFECT);
    obj->control = DartEffect_Control;
    obj->draw_routine = Object_DrawSpriteItem;
    obj->semi_transparent = 1;
}
