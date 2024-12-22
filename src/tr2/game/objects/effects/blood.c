#include "game/objects/effects/blood.h"

#include "game/effects.h"
#include "game/math.h"
#include "game/objects/common.h"
#include "global/vars.h"

void __cdecl Blood_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    const OBJECT *const obj = Object_GetObject(fx->object_id);
    fx->pos.x += (fx->speed * Math_Sin(fx->rot.y)) >> W2V_SHIFT;
    fx->pos.z += (fx->speed * Math_Cos(fx->rot.y)) >> W2V_SHIFT;
    fx->counter++;
    if (fx->counter == 4) {
        fx->frame_num--;
        fx->counter = 0;
        if (fx->frame_num <= obj->mesh_count) {
            Effect_Kill(fx_num);
        }
    }
}

void Blood_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BLOOD);
    obj->control = Blood_Control;
    obj->semi_transparent = 1;
}
