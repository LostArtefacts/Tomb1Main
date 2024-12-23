#include "game/objects/effects/blood.h"

#include "game/effects.h"
#include "global/const.h"
#include "global/vars.h"
#include "math/math.h"

void Blood_Setup(OBJECT *obj)
{
    obj->control = Blood_Control;
}

void Blood_Control(int16_t effect_num)
{
    EFFECT *effect = &g_Effects[effect_num];
    effect->pos.x += (Math_Sin(effect->rot.y) * effect->speed) >> W2V_SHIFT;
    effect->pos.z += (Math_Cos(effect->rot.y) * effect->speed) >> W2V_SHIFT;
    effect->counter++;
    if (effect->counter == 4) {
        effect->counter = 0;
        effect->frame_num--;
        if (effect->frame_num <= g_Objects[effect->object_id].nmeshes) {
            Effect_Kill(effect_num);
        }
    }
}
