#include "game/objects/effects/dart_effect.h"

#include "game/effects.h"
#include "game/objects/common.h"
#include "global/vars.h"

void __cdecl DartEffect_Control(int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    const OBJECT *const obj = Object_GetObject(effect->object_id);

    effect->counter++;
    if (effect->counter >= 3) {
        effect->frame_num--;
        effect->counter = 0;
        if (effect->frame_num <= obj->mesh_count) {
            Effect_Kill(effect_num);
        }
    }
}

void DartEffect_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DART_EFFECT);
    obj->control = DartEffect_Control;
    obj->draw_routine = Object_DrawSpriteItem;
    obj->semi_transparent = 1;
}
