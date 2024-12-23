#include "game/objects/effects/explosion.h"

#include "game/effects.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "global/vars.h"

void __cdecl Explosion_Control(const int16_t effect_num)
{
    EFFECT *const effect = &g_Effects[effect_num];
    const OBJECT *const obj = Object_GetObject(effect->object_id);
    effect->counter++;
    if (effect->counter == 2) {
        effect->frame_num--;
        effect->counter = 0;
        if (effect->frame_num > obj->mesh_count) {
            Output_AddDynamicLight(
                effect->pos.x, effect->pos.y, effect->pos.z, 13, 11);
        } else {
            Effect_Kill(effect_num);
        }
    } else {
        Output_AddDynamicLight(
            effect->pos.x, effect->pos.y, effect->pos.z, 12, 10);
    }
}

void Explosion_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_EXPLOSION);
    obj->control = Explosion_Control;
    obj->semi_transparent = 1;
}
