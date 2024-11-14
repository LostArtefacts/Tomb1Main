#include "game/objects/general/window.h"

#include "game/box.h"
#include "game/objects/common.h"
#include "game/room.h"
#include "global/funcs.h"
#include "global/vars.h"

void Window_1_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WINDOW_1);
    obj->initialise = Window_Initialise;
    obj->collision = Object_Collision;
    obj->control = Window_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

void Window_2_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WINDOW_2);
    obj->initialise = Window_Initialise;
    obj->collision = Object_Collision;
    obj->control = SmashIce_Control;
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
