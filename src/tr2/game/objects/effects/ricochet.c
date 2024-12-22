#include "game/objects/effects/ricochet.h"

#include "game/effects.h"
#include "global/vars.h"

void __cdecl Richochet_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    fx->counter--;
    if (fx->counter == 0) {
        Effect_Kill(fx_num);
    }
}

void Ricochet_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_RICOCHET);
    obj->control = Richochet_Control;
}
