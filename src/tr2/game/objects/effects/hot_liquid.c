#include "game/objects/effects/hot_liquid.h"

#include "game/effects.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

void __cdecl HotLiquid_Control(const int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    OBJECT *const obj = Object_GetObject(O_HOT_LIQUID);

    fx->frame_num--;
    if (fx->frame_num <= obj->mesh_count) {
        fx->frame_num = 0;
    }

    fx->pos.y += fx->fall_speed;
    fx->fall_speed += GRAVITY;

    int16_t room_num = fx->room_num;
    const SECTOR *const sector =
        Room_GetSector(fx->pos.x, fx->pos.y, fx->pos.z, &room_num);
    const int32_t height =
        Room_GetHeight(sector, fx->pos.x, fx->pos.y, fx->pos.z);

    if (fx->pos.y >= height) {
        Sound_Effect(SFX_WATERFALL_2, &fx->pos, SPM_NORMAL);
        fx->object_id = O_SPLASH;
        fx->pos.y = height;
        fx->rot.y = 2 * Random_GetDraw();
        fx->fall_speed = 0;
        fx->speed = 50;
        return;
    }

    if (fx->room_num != room_num) {
        Effect_NewRoom(fx_num, room_num);
    }
    Sound_Effect(SFX_BOWL_POUR, &fx->pos, SPM_NORMAL);
}

void HotLiquid_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_HOT_LIQUID);
    obj->control = HotLiquid_Control;
    obj->semi_transparent = 1;
}
