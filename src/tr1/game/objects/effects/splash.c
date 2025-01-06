#include "game/objects/effects/splash.h"

#include "game/effects.h"
#include "game/objects/common.h"

#include <libtrx/game/math.h>

void Splash_Setup(OBJECT *obj)
{
    obj->control = Splash_Control;
}

void Splash_Control(int16_t effect_num)
{
    EFFECT *effect = Effect_Get(effect_num);
    const OBJECT *const object = Object_GetObject(effect->object_id);

    effect->frame_num--;
    if (effect->frame_num <= object->nmeshes) {
        Effect_Kill(effect_num);
        return;
    }

    effect->pos.x += (effect->speed * Math_Sin(effect->rot.y)) >> W2V_SHIFT;
    effect->pos.z += (effect->speed * Math_Cos(effect->rot.y)) >> W2V_SHIFT;
}
