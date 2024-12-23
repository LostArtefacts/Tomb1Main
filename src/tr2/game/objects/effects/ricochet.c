#include "game/objects/effects/ricochet.h"

#include "game/effects.h"
#include "global/vars.h"

void __cdecl Ricochet_Control(const int16_t effect_num)
{
    EFFECT *const effect = &g_Effects[effect_num];
    effect->counter--;
    if (effect->counter == 0) {
        Effect_Kill(effect_num);
    }
}

void Ricochet_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_RICOCHET);
    obj->control = Ricochet_Control;
}
