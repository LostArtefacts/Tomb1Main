#include "game/objects/effects/twinkle.h"

#include "game/collide.h"
#include "game/effects.h"
#include "game/random.h"
#include "global/const.h"
#include "global/vars.h"

void Twinkle_Setup(OBJECT *obj)
{
    obj->control = Twinkle_Control;
}

void Twinkle_Control(int16_t effect_num)
{
    EFFECT *effect = &g_Effects[effect_num];
    effect->counter++;
    if (effect->counter == 1) {
        effect->counter = 0;
        effect->frame_num--;
        if (effect->frame_num <= g_Objects[effect->object_id].nmeshes) {
            Effect_Kill(effect_num);
        }
    }
}

void Twinkle_Spawn(GAME_VECTOR *pos)
{
    int16_t effect_num = Effect_Create(pos->room_num);
    if (effect_num != NO_EFFECT) {
        EFFECT *effect = &g_Effects[effect_num];
        effect->pos.x = pos->x;
        effect->pos.y = pos->y;
        effect->pos.z = pos->z;
        effect->counter = 0;
        effect->object_id = O_TWINKLE;
        effect->frame_num = 0;
    }
}

void Twinkle_SparkleItem(ITEM *item, int mesh_mask)
{
    SPHERE slist[34];
    GAME_VECTOR effect_pos;

    int32_t num = Collide_GetSpheres(item, slist, 1);
    effect_pos.room_num = item->room_num;
    for (int i = 0; i < num; i++) {
        if (mesh_mask & (1 << i)) {
            SPHERE *sptr = &slist[i];
            effect_pos.x =
                sptr->x + sptr->r * (Random_GetDraw() - 0x4000) / 0x4000;
            effect_pos.y =
                sptr->y + sptr->r * (Random_GetDraw() - 0x4000) / 0x4000;
            effect_pos.z =
                sptr->z + sptr->r * (Random_GetDraw() - 0x4000) / 0x4000;
            Twinkle_Spawn(&effect_pos);
        }
    }
}
