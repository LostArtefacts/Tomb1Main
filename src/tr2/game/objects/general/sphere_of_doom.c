#include "game/objects/general/sphere_of_doom.h"

#include "game/math.h"
#include "game/room.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#define SPHERE_OF_DOOM_RADIUS (STEP_L * 5 / 2) // = 640

void __cdecl SphereOfDoom_Collision(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    if (Room_Get(lara_item->room_num)->flags & RF_UNDERWATER) {
        return;
    }

    const ITEM *const item = Item_Get(item_num);
    const int32_t dx = lara_item->pos.x - item->pos.x;
    const int32_t dz = lara_item->pos.z - item->pos.z;
    const int32_t radius = (SPHERE_OF_DOOM_RADIUS * item->timer) >> 8;

    if (SQUARE(dx) + SQUARE(dz) >= SQUARE(radius)) {
        return;
    }

    const int16_t angle = Math_Atan(dz, dx);
    const int16_t diff = lara_item->rot.y - angle;
    if (ABS(diff) < PHD_90) {
        lara_item->speed = 150;
        lara_item->rot.y = angle;
    } else {
        lara_item->speed = -150;
        lara_item->rot.y = angle + PHD_180;
    }

    lara_item->gravity = 1;
    lara_item->fall_speed = -50;
    lara_item->pos.x =
        item->pos.x + (((radius + 50) * Math_Sin(angle)) >> W2V_SHIFT);
    lara_item->pos.z =
        item->pos.z + (((radius + 50) * Math_Cos(angle)) >> W2V_SHIFT);
    lara_item->rot.x = 0;
    lara_item->rot.z = 0;
    lara_item->anim_num = LA_FALL_START;
    lara_item->frame_num = g_Anims[lara_item->anim_num].frame_base;
    lara_item->current_anim_state = LS_FORWARD_JUMP;
    lara_item->goal_anim_state = LS_FORWARD_JUMP;
}

void SphereOfDoom_Setup(OBJECT *const obj, const bool transparent)
{
    obj->collision = SphereOfDoom_Collision;
    obj->control = SphereOfDoom_Control;
    obj->draw_routine = SphereOfDoom_Draw;
    obj->save_position = 1;
    obj->save_flags = 1;
    if (transparent) {
        obj->semi_transparent = 1;
    }
}
