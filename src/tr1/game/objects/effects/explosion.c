#include "game/objects/effects/explosion.h"

#include "game/effects.h"
#include "global/vars.h"

void Explosion_Setup(OBJECT *obj)
{
    obj->control = Explosion_Control;
}

void Explosion_Control(int16_t effect_num)
{
    EFFECT *effect = &g_Effects[effect_num];
    effect->counter++;
    if (effect->counter == 2) {
        effect->counter = 0;
        effect->frame_num--;
        if (effect->frame_num <= g_Objects[effect->object_id].nmeshes) {
            Effect_Kill(effect_num);
        }
    }
}
