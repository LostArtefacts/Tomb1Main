#include "game/objects/effects/water_sprite.h"

#include "game/effects.h"
#include "game/math.h"
#include "global/vars.h"

void __cdecl WaterSprite_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    const OBJECT *const obj = Object_GetObject(fx->object_id);

    fx->counter--;
    if (fx->counter % 4 == 0) {
        fx->frame_num--;
        if (fx->frame_num <= obj->mesh_count) {
            fx->frame_num = 0;
        }
    }

    if (fx->counter == 0 || fx->fall_speed > 0) {
        Effect_Kill(fx_num);
        return;
    }

    fx->pos.x += (fx->speed * Math_Sin(fx->rot.y)) >> W2V_SHIFT;
    fx->pos.z += (fx->speed * Math_Cos(fx->rot.y)) >> W2V_SHIFT;
    if (fx->fall_speed != 0) {
        fx->pos.y += fx->fall_speed;
        fx->fall_speed += GRAVITY;
    }
}

void WaterSprite_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WATER_SPRITE);
    obj->control = WaterSprite_Control;
    obj->semi_transparent = 1;
}
