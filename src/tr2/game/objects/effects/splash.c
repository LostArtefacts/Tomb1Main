#include "game/objects/effects/splash.h"

#include "game/effects.h"
#include "game/math.h"
#include "game/objects/common.h"
#include "global/vars.h"

void Spawn_Splash_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SPLASH);
    obj->control = Spawn_Splash_Control;
    obj->semi_transparent = 1;
}

void __cdecl Spawn_Splash_Control(const int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    const OBJECT *const object = Object_GetObject(effect->object_id);

    effect->frame_num--;
    if (effect->frame_num <= object->mesh_count) {
        Effect_Kill(effect_num);
        return;
    }

    effect->pos.x += (effect->speed * Math_Sin(effect->rot.y)) >> W2V_SHIFT;
    effect->pos.z += (effect->speed * Math_Cos(effect->rot.y)) >> W2V_SHIFT;
}
