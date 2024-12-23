#include "game/objects/effects/gun_flash.h"

#include "game/effects.h"
#include "game/output.h"
#include "game/random.h"
#include "global/vars.h"

void __cdecl GunFlash_Control(const int16_t effect_num)
{
    EFFECT *const effect = &g_Effects[effect_num];

    effect->counter--;
    if (effect->counter == 0) {
        Effect_Kill(effect_num);
        return;
    }

    effect->rot.z = Random_GetControl();
    Output_AddDynamicLight(effect->pos.x, effect->pos.y, effect->pos.z, 12, 11);
}

void GunFlash_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GUN_FLASH);
    obj->control = GunFlash_Control;
}
