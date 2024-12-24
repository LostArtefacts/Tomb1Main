#include "game/objects/effects/gunshot.h"

#include "game/effects.h"
#include "game/random.h"

void GunShot_Setup(OBJECT *obj)
{
    obj->control = GunShot_Control;
}

void GunShot_Control(int16_t effect_num)
{
    EFFECT *effect = Effect_Get(effect_num);
    effect->counter--;
    if (!effect->counter) {
        Effect_Kill(effect_num);
        return;
    }
    effect->rot.z = Random_GetControl();
}
