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

void __cdecl Splash_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    fx->frame_num--;

    if (fx->frame_num > g_Objects[fx->object_id].mesh_count) {
        fx->pos.x += (fx->speed * Math_Sin(fx->rot.y)) >> W2V_SHIFT;
        fx->pos.z += (fx->speed * Math_Cos(fx->rot.y)) >> W2V_SHIFT;
    } else {
        Effect_Kill(fx_num);
    }
}
