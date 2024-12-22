#include "game/objects/general/window.h"

#include "game/box.h"
#include "game/items.h"
#include "game/lara/misc.h"
#include "game/math.h"
#include "game/objects/common.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#include <libtrx/utils.h>

void Window_1_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WINDOW_1);
    obj->initialise = Window_Initialise;
    obj->collision = Object_Collision;
    obj->control = Window_1_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

void Window_2_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WINDOW_2);
    obj->initialise = Window_Initialise;
    obj->collision = Object_Collision;
    obj->control = Window_2_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

void __cdecl Window_Initialise(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    item->flags = 0;
    item->mesh_bits = 1;

    const ROOM *const r = Room_Get(item->room_num);
    const int32_t z_sector = (item->pos.z - r->pos.z) >> WALL_SHIFT;
    const int32_t x_sector = (item->pos.x - r->pos.x) >> WALL_SHIFT;
    const SECTOR *const sector = &r->sectors[z_sector + x_sector * r->size.z];
    BOX_INFO *const box = &g_Boxes[sector->box];

    if (box->overlap_index & BOX_BLOCKABLE) {
        box->overlap_index |= BOX_BLOCKED;
    }
}

void __cdecl Window_1_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (item->flags & IF_ONE_SHOT) {
        return;
    }

    if (g_Lara.skidoo != NO_ITEM) {
        if (Lara_IsNearItem(&item->pos, 512)) {
            Window_Smash(item_num);
        }
    } else if (item->touch_bits) {
        item->touch_bits = 0;
        const int32_t speed =
            ABS((g_LaraItem->speed * Math_Cos(g_LaraItem->rot.y - item->rot.y))
                >> W2V_SHIFT);
        if (speed >= 50) {
            Window_Smash(item_num);
        }
    }
}

void __cdecl Window_2_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (item->flags & IF_ONE_SHOT) {
        return;
    }

    const ROOM *const r = Room_Get(item->room_num);
    const int32_t z_sector = (item->pos.z - r->pos.z) >> WALL_SHIFT;
    const int32_t x_sector = (item->pos.x - r->pos.x) >> WALL_SHIFT;
    const SECTOR *const sector = &r->sectors[z_sector + x_sector * r->size.z];
    BOX_INFO *const box = &g_Boxes[sector->box];

    if (box->overlap_index & BOX_BLOCKED) {
        box->overlap_index &= ~BOX_BLOCKED;
    }

    item->mesh_bits = ~1;
    item->collidable = 0;
    Effect_ExplodingDeath(item_num, 65278, 0);
    Sound_Effect(SFX_BRITTLE_GROUND_BREAK, &item->pos, SPM_NORMAL);

    item->flags |= IF_ONE_SHOT;
    item->status = IS_DEACTIVATED;
    Item_RemoveActive(item_num);
}

void __cdecl Window_Smash(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    const ROOM *const r = Room_Get(item->room_num);
    const int32_t z_sector = (item->pos.z - r->pos.z) >> WALL_SHIFT;
    const int32_t x_sector = (item->pos.x - r->pos.x) >> WALL_SHIFT;
    const SECTOR *const sector = &r->sectors[z_sector + x_sector * r->size.z];
    BOX_INFO *const box = &g_Boxes[sector->box];

    if (box->overlap_index & BOX_BLOCKABLE) {
        box->overlap_index &= ~BOX_BLOCKED;
    }

    item->collidable = 0;
    item->mesh_bits = ~1;
    Effect_ExplodingDeath(item_num, 65278, 0);
    Sound_Effect(SFX_GLASS_BREAK, &item->pos, SPM_NORMAL);

    item->flags |= IF_ONE_SHOT;
    if (item->status == IS_ACTIVE) {
        Item_RemoveActive(item_num);
    }
    item->status = IS_DEACTIVATED;
}
