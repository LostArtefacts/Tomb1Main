#include "game/objects/effects/glow.h"

#include "game/effects.h"
#include "global/vars.h"

void __cdecl Glow_Control(const int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);

    effect->counter--;
    if (effect->counter == 0) {
        Effect_Kill(effect_num);
        return;
    }

    effect->shade += effect->speed;
    effect->frame_num += effect->fall_speed;
}

void Glow_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GLOW);
    obj->control = Glow_Control;
}
