#include "game/objects/effects/glow.h"

#include "game/effects.h"
#include "global/vars.h"

void __cdecl Glow_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];

    fx->counter--;
    if (fx->counter == 0) {
        Effect_Kill(fx_num);
        return;
    }

    fx->shade += fx->speed;
    fx->frame_num += fx->fall_speed;
}

void Glow_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GLOW);
    obj->control = Glow_Control;
}
