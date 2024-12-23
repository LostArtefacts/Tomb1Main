#include "game/objects/effects/natla_gun.h"

#include "game/effects.h"
#include "game/room.h"
#include "global/vars.h"
#include "math/math.h"

void NatlaGun_Setup(OBJECT *obj)
{
    obj->control = NatlaGun_Control;
}

void NatlaGun_Control(int16_t effect_num)
{
    EFFECT *effect = &g_Effects[effect_num];
    OBJECT *object = &g_Objects[effect->object_id];

    effect->frame_num--;
    if (effect->frame_num <= object->nmeshes) {
        Effect_Kill(effect_num);
    }

    if (effect->frame_num == -1) {
        return;
    }

    int32_t z = effect->pos.z
        + ((effect->speed * Math_Cos(effect->rot.y)) >> W2V_SHIFT);
    int32_t x = effect->pos.x
        + ((effect->speed * Math_Sin(effect->rot.y)) >> W2V_SHIFT);
    int32_t y = effect->pos.y;
    int16_t room_num = effect->room_num;
    const SECTOR *const sector = Room_GetSector(x, y, z, &room_num);

    if (y >= Room_GetHeight(sector, x, y, z)
        || y <= Room_GetCeiling(sector, x, y, z)) {
        return;
    }

    effect_num = Effect_Create(room_num);
    if (effect_num != NO_ITEM) {
        EFFECT *newfx = &g_Effects[effect_num];
        newfx->pos.x = x;
        newfx->pos.y = y;
        newfx->pos.z = z;
        newfx->rot.y = effect->rot.y;
        newfx->room_num = room_num;
        newfx->speed = effect->speed;
        newfx->frame_num = 0;
        newfx->object_id = O_MISSILE_1;
    }
}
