#include "game/objects/traps/hook.h"

#include "game/creature.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/objects/common.h"
#include "global/funcs.h"

#include <libtrx/game/lara/common.h>

#define HOOK_DAMAGE 50

void __cdecl Hook_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    if (item->touch_bits != 0) {
        const ITEM *const lara_item = Lara_GetItem();
        Lara_TakeDamage(HOOK_DAMAGE, true);
        DoLotsOfBlood(
            lara_item->pos.x, lara_item->pos.y - WALL_L / 2, lara_item->pos.z,
            lara_item->speed, lara_item->rot.y, lara_item->room_num, 3);
    }

    Item_Animate(item);
}

void Hook_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_HOOK);
    obj->control = Hook_Control;
    obj->collision = Creature_Collision;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
