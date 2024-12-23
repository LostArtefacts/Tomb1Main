#include "game/objects/effects/splash.h"

#include "game/effects.h"
#include "game/math.h"
#include "global/vars.h"

void Splash_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SPLASH);
    obj->control = Splash_Control;
    obj->semi_transparent = 1;
}

void __cdecl Splash_Control(const int16_t effect_num)
{
    EFFECT *const effect = &g_Effects[effect_num];
    effect->frame_num--;

    if (effect->frame_num > g_Objects[effect->object_id].mesh_count) {
        effect->pos.x += (effect->speed * Math_Sin(effect->rot.y)) >> W2V_SHIFT;
        effect->pos.z += (effect->speed * Math_Cos(effect->rot.y)) >> W2V_SHIFT;
    } else {
        Effect_Kill(effect_num);
    }
}
