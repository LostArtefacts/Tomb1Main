#include "game/objects/general/grenade.h"

#include "game/creature.h"
#include "game/effects.h"
#include "game/gun/gun_misc.h"
#include "game/items.h"
#include "game/math.h"
#include "game/objects/general/window.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#define M_BLAST_RADIUS (WALL_L / 2) // = 512
#define M_SPEED 200
#define M_FALL_SPEED (M_SPEED - 10) // = 190

static void M_Explode(int16_t grenade_item_num, XYZ_32 pos);

static void M_Explode(int16_t grenade_item_num, const XYZ_32 pos)
{
    const ITEM *const grenade_item = Item_Get(grenade_item_num);
    const int16_t fx_num = Effect_Create(grenade_item->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->pos = pos;
        fx->speed = 0;
        fx->frame_num = 0;
        fx->counter = 0;
        fx->object_id = O_EXPLOSION;
    }
    Sound_Effect(SFX_EXPLOSION_1, NULL, SPM_NORMAL);
    Item_Kill(grenade_item_num);
}

void Grenade_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_GRENADE);
    obj->control = Grenade_Control;
    obj->save_position = 1;
}

void __cdecl Grenade_Control(int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    const XYZ_32 old_pos = item->pos;

    item->speed--;
    if (item->speed < M_FALL_SPEED) {
        item->fall_speed++;
    }
    item->pos.y +=
        item->fall_speed - ((item->speed * Math_Sin(item->rot.x)) >> W2V_SHIFT);

    const int16_t speed = (item->speed * Math_Cos(item->rot.x)) >> W2V_SHIFT;
    item->pos.z += (speed * Math_Cos(item->rot.y)) >> W2V_SHIFT;
    item->pos.x += (speed * Math_Sin(item->rot.y)) >> W2V_SHIFT;

    int16_t room_num = item->room_num;
    const SECTOR *const sector =
        Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);
    item->floor = Room_GetHeight(sector, item->pos.x, item->pos.y, item->pos.z);
    if (item->room_num != room_num) {
        Item_NewRoom(item_num, room_num);
    }

    bool explode = false;
    int32_t radius = 0;
    if (item->pos.y >= item->floor
        || item->pos.y
            <= Room_GetCeiling(sector, item->pos.x, item->pos.y, item->pos.z)) {
        radius = M_BLAST_RADIUS;
        explode = true;
    }

    for (int16_t target_item_num = Room_Get(item->room_num)->item_num;
         target_item_num != NO_ITEM;
         target_item_num = Item_Get(target_item_num)->next_item) {
        ITEM *const target_item = Item_Get(target_item_num);
        const OBJECT *const target_obj =
            Object_GetObject(target_item->object_id);
        if (target_item == g_LaraItem) {
            continue;
        }
        if (!target_item->collidable) {
            continue;
        }

        if (target_item->object_id != O_WINDOW_1
            && (!target_obj->intelligent || target_item->status == IS_INVISIBLE
                || target_obj->collision == NULL)) {
            continue;
        }

        const FRAME_INFO *const frame = Item_GetBestFrame(target_item);
        const BOUNDS_16 *const bounds = &frame->bounds;

        const int32_t cdy = item->pos.y - target_item->pos.y;
        if (cdy + radius < bounds->min_y || cdy - radius > bounds->max_y) {
            continue;
        }

        const int32_t cy = Math_Cos(target_item->rot.y);
        const int32_t sy = Math_Sin(target_item->rot.y);
        const int32_t cdx = item->pos.x - target_item->pos.x;
        const int32_t cdz = item->pos.z - target_item->pos.z;
        const int32_t odx = old_pos.x - target_item->pos.x;
        const int32_t odz = old_pos.z - target_item->pos.z;

        const int32_t rx = (cy * cdx - sy * cdz) >> W2V_SHIFT;
        const int32_t sx = (cy * odx - sy * odz) >> W2V_SHIFT;
        if ((rx + radius < bounds->min_x && sx + radius < bounds->min_x)
            || (rx - radius > bounds->max_x && sx - radius > bounds->max_x)) {
            continue;
        }

        const int32_t rz = (sy * cdx + cy * cdz) >> W2V_SHIFT;
        const int32_t sz = (sy * odx + cy * odz) >> W2V_SHIFT;
        if ((rz + radius < bounds->min_z && sz + radius < bounds->min_z)
            || (rz - radius > bounds->max_z && sz - radius > bounds->max_z)) {
            continue;
        }

        if (target_item->object_id == O_WINDOW_1) {
            Window_Smash(target_item_num);
        } else {
            explode = true;

            if (!target_obj->intelligent || target_item->status != IS_ACTIVE) {
                continue;
            }

            Gun_HitTarget(target_item, NULL, 30);
            g_SaveGame.statistics.hits++;

            if (target_item->hit_points <= 0) {
                if (target_item->object_id != O_DRAGON_FRONT
                    && target_item->object_id != O_BIRD_GUARDIAN) {
                    Creature_Die(target_item_num, true);
                }
            }
        }
    }

    if (explode) {
        M_Explode(item_num, old_pos);
    }
}
