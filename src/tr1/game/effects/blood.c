#include "game/effects/blood.h"

#include "game/effects.h"
#include "global/const.h"
#include "global/types.h"

int16_t Effect_Blood(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t direction,
    int16_t room_num)
{
    int16_t effect_num = Effect_Create(room_num);
    if (effect_num != NO_ITEM) {
        EFFECT *effect = &g_Effects[effect_num];
        effect->pos.x = x;
        effect->pos.y = y;
        effect->pos.z = z;
        effect->rot.y = direction;
        effect->object_id = O_BLOOD_1;
        effect->frame_num = 0;
        effect->counter = 0;
        effect->speed = speed;
    }
    return effect_num;
}
