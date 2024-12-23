#include "game/effects/gun.h"

#include "game/effects.h"
#include "game/items.h"
#include "game/random.h"
#include "global/const.h"
#include "global/vars.h"
#include "math/math.h"

#include <libtrx/utils.h>

#define SHARD_SPEED 250
#define ROCKET_SPEED 220

static void M_ShootAtLara(EFFECT *effect);

void M_ShootAtLara(EFFECT *effect)
{
    int32_t x = g_LaraItem->pos.x - effect->pos.x;
    int32_t y = g_LaraItem->pos.y - effect->pos.y;
    int32_t z = g_LaraItem->pos.z - effect->pos.z;

    const BOUNDS_16 *const bounds = Item_GetBoundsAccurate(g_LaraItem);
    y += bounds->max.y + (bounds->min.y - bounds->max.y) * 3 / 4;

    int32_t dist = Math_Sqrt(SQUARE(x) + SQUARE(z));
    effect->rot.x = -(PHD_ANGLE)Math_Atan(dist, y);
    effect->rot.y = Math_Atan(z, x);
    effect->rot.x += (Random_GetControl() - 0x4000) / 0x40;
    effect->rot.y += (Random_GetControl() - 0x4000) / 0x40;
}

int16_t Effect_ShardGun(
    int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE y_rot,
    int16_t room_num)
{
    int16_t effect_num = Effect_Create(room_num);
    if (effect_num != NO_ITEM) {
        EFFECT *effect = &g_Effects[effect_num];
        effect->room_num = room_num;
        effect->pos.x = x;
        effect->pos.y = y;
        effect->pos.z = z;
        effect->rot.x = 0;
        effect->rot.y = y_rot;
        effect->rot.z = 0;
        effect->object_id = O_MISSILE_2;
        effect->frame_num = 0;
        effect->speed = SHARD_SPEED;
        effect->shade = 3584;
        M_ShootAtLara(effect);
    }
    return effect_num;
}

int16_t Effect_RocketGun(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num)
{
    int16_t effect_num = Effect_Create(room_num);
    if (effect_num != NO_ITEM) {
        EFFECT *effect = &g_Effects[effect_num];
        effect->room_num = room_num;
        effect->pos.x = x;
        effect->pos.y = y;
        effect->pos.z = z;
        effect->rot.x = 0;
        effect->rot.y = y_rot;
        effect->rot.z = 0;
        effect->object_id = O_MISSILE_3;
        effect->frame_num = 0;
        effect->speed = ROCKET_SPEED;
        effect->shade = HIGH_LIGHT;
        M_ShootAtLara(effect);
    }
    return effect_num;
}
