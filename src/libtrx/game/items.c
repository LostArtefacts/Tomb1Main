#include "game/items.h"

#include "game/const.h"
#include "game/objects/common.h"
#include "utils.h"

#include <stddef.h>

void Item_TakeDamage(
    ITEM *const item, const int16_t damage, const bool hit_status)
{
#if TR_VERSION == 1
    if (item->hit_points == DONT_TARGET) {
        return;
    }
#endif

    item->hit_points -= damage;
    CLAMPL(item->hit_points, 0);

    if (hit_status) {
        item->hit_status = 1;
    }
}

ITEM *Item_Find(const GAME_OBJECT_ID object_id)
{
    for (int32_t item_num = 0; item_num < Item_GetTotalCount(); item_num++) {
        ITEM *const item = Item_Get(item_num);
        if (item->object_id == object_id) {
            return item;
        }
    }

    return NULL;
}

bool Item_TestAnimEqual(const ITEM *const item, const int16_t anim_idx)
{
    const OBJECT *const object = Object_GetObject(item->object_id);
    return item->anim_num == object->anim_idx + anim_idx;
}

void Item_SwitchToAnim(
    ITEM *const item, const int16_t anim_idx, const int16_t frame)
{
    Item_SwitchToObjAnim(item, anim_idx, frame, item->object_id);
}
