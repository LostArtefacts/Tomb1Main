#include "game/objects/effects/splash.h"

#include "game/effects.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/vars.h"
#include "math/math.h"

void Splash_Setup(OBJECT *obj)
{
    obj->control = Splash_Control;
}

void Splash_Control(int16_t effect_num)
{
    EFFECT *effect = &g_Effects[effect_num];
    effect->frame_num--;
    if (effect->frame_num <= g_Objects[effect->object_id].nmeshes) {
        Effect_Kill(effect_num);
        return;
    }

    effect->pos.z += (Math_Cos(effect->rot.y) * effect->speed) >> W2V_SHIFT;
    effect->pos.x += (Math_Sin(effect->rot.y) * effect->speed) >> W2V_SHIFT;
}

void Splash_Spawn(ITEM *item)
{
    int16_t wh = Room_GetWaterHeight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);
    int16_t room_num = item->room_num;
    Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);

    Sound_Effect(SFX_LARA_SPLASH, &item->pos, SPM_NORMAL);

    for (int i = 0; i < 10; i++) {
        int16_t effect_num = Effect_Create(room_num);
        if (effect_num != NO_ITEM) {
            EFFECT *effect = &g_Effects[effect_num];
            effect->pos.x = item->pos.x;
            effect->pos.y = wh;
            effect->pos.z = item->pos.z;
            effect->rot.y = PHD_180 + 2 * Random_GetDraw();
            effect->object_id = O_SPLASH_1;
            effect->frame_num = 0;
            effect->speed = Random_GetDraw() / 256;
        }
    }
}
