#include "game/objects/effects/gun_flash.h"

#include "game/effects.h"
#include "game/output.h"
#include "game/random.h"
#include "global/vars.h"

void __cdecl GunFlash_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];

    fx->counter--;
    if (fx->counter == 0) {
        Effect_Kill(fx_num);
        return;
    }

    fx->rot.z = Random_GetControl();
    Output_AddDynamicLight(fx->pos.x, fx->pos.y, fx->pos.z, 12, 11);
}

void GunFlash_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GUN_FLASH);
    obj->control = GunFlash_Control;
}
