#include "game/objects/effects/snow_sprite.h"

#include "game/effects.h"
#include "game/math.h"
#include "global/vars.h"

void __cdecl SnowSprite_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    const OBJECT *const obj = Object_GetObject(fx->object_id);

    fx->frame_num--;
    if (fx->frame_num <= obj->mesh_count) {
        Effect_Kill(fx_num);
        return;
    }

    fx->pos.z += (fx->speed * Math_Cos(fx->rot.y)) >> W2V_SHIFT;
    fx->pos.x += (fx->speed * Math_Sin(fx->rot.y)) >> W2V_SHIFT;
    if (fx->fall_speed != 0) {
        fx->pos.y += fx->fall_speed;
        fx->fall_speed += GRAVITY;
    }
}

void SnowSprite_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SNOW_SPRITE);
    obj->control = SnowSprite_Control;
}
