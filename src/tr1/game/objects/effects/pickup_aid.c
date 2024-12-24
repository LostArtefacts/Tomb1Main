#include "game/objects/effects/pickup_aid.h"

#include "game/effects.h"

static void M_Control(int16_t effect_num);

static void M_Control(int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    effect->counter++;
    if (effect->counter == 1) {
        effect->counter = 0;
        effect->frame_num--;
        if (effect->frame_num <= Object_GetObject(effect->object_id)->nmeshes) {
            Effect_Kill(effect_num);
        }
    }
}

void PickupAid_Setup(OBJECT *const obj)
{
    obj->control = M_Control;
}

void PickupAid_Spawn(const GAME_VECTOR *const pos)
{
    const int16_t effect_num = Effect_Create(pos->room_num);
    if (effect_num != NO_EFFECT) {
        EFFECT *const effect = Effect_Get(effect_num);
        effect->pos.x = pos->x;
        effect->pos.y = pos->y;
        effect->pos.z = pos->z;
        effect->counter = 0;
        effect->object_id = O_PICKUP_AID;
        effect->frame_num = 0;
    }
}
