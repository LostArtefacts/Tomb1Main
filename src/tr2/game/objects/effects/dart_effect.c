#include "game/objects/effects/dart_effect.h"

#include "game/effects.h"
#include "game/objects/common.h"
#include "global/vars.h"

void __cdecl DartEffect_Control(int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    const OBJECT *const obj = Object_GetObject(fx->object_id);

    fx->counter++;
    if (fx->counter >= 3) {
        fx->frame_num--;
        fx->counter = 0;
        if (fx->frame_num <= obj->mesh_count) {
            Effect_Kill(fx_num);
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
