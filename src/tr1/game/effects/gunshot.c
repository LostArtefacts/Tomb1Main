#include "game/effects/gunshot.h"

#include "game/collide.h"
#include "game/effects.h"
#include "game/effects/blood.h"
#include "game/objects/effects/ricochet.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/vars.h"

int16_t Effect_GunShot(
    int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE y_rot,
    int16_t room_num)
{
    int16_t effect_num = Effect_Create(room_num);
    if (effect_num != NO_ITEM) {
        EFFECT *effect = &g_Effects[effect_num];
        effect->pos.x = x;
        effect->pos.y = y;
        effect->pos.z = z;
        effect->room_num = room_num;
        effect->rot.x = 0;
        effect->rot.y = y_rot;
        effect->rot.z = 0;
        effect->counter = 3;
        effect->frame_num = 0;
        effect->object_id = O_GUN_FLASH;
        effect->shade = HIGH_LIGHT;
    }
    return effect_num;
}

int16_t Effect_GunShotHit(
    int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE y_rot,
    int16_t room_num)
{
    XYZ_32 pos = {
        .x = 0,
        .y = 0,
        .z = 0,
    };
    Collide_GetJointAbsPosition(
        g_LaraItem, &pos, (Random_GetControl() * 25) / 0x7FFF);
    Effect_Blood(
        pos.x, pos.y, pos.z, g_LaraItem->speed, g_LaraItem->rot.y,
        g_LaraItem->room_num);
    Sound_Effect(SFX_LARA_BULLETHIT, &g_LaraItem->pos, SPM_NORMAL);
    return Effect_GunShot(x, y, z, speed, y_rot, room_num);
}

int16_t Effect_GunShotMiss(
    int32_t x, int32_t y, int32_t z, int16_t speed, PHD_ANGLE y_rot,
    int16_t room_num)
{
    GAME_VECTOR pos;
    pos.x = g_LaraItem->pos.x
        + ((Random_GetDraw() - 0x4000) * (WALL_L / 2)) / 0x7FFF;
    pos.y = g_LaraItem->floor;
    pos.z = g_LaraItem->pos.z
        + ((Random_GetDraw() - 0x4000) * (WALL_L / 2)) / 0x7FFF;
    pos.room_num = g_LaraItem->room_num;
    Ricochet_Spawn(&pos);
    return Effect_GunShot(x, y, z, speed, y_rot, room_num);
}
