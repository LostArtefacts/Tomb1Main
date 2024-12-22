#include "game/objects/effects/explosion.h"

#include "game/effects.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "global/vars.h"

void __cdecl Explosion_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    const OBJECT *const obj = Object_GetObject(fx->object_id);
    fx->counter++;
    if (fx->counter == 2) {
        fx->frame_num--;
        fx->counter = 0;
        if (fx->frame_num > obj->mesh_count) {
            Output_AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 13, 11);
        } else {
            Effect_Kill(fx_num);
        }
    } else {
        Output_AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 12, 10);
    }
}

void Explosion_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_EXPLOSION);
    obj->control = Explosion_Control;
    obj->semi_transparent = 1;
}
