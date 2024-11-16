#include "game/objects/general/trapdoor.h"

#include "game/items.h"

typedef enum {
    TRAPDOOR_STATE_CLOSED,
    TRAPDOOR_STATE_OPEN,
} TRAPDOOR_STATE;

void Trapdoor_Setup(OBJECT *const obj)
{
    obj->control = Trapdoor_Control;
    obj->ceiling = Trapdoor_Ceiling;
    obj->floor = Trapdoor_Floor;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

int32_t __cdecl Trapdoor_IsItemOnTop(
    const ITEM *const item, const int32_t x, const int32_t z)
{
    const BOUNDS_16 *const orig_bounds = &Item_GetBestFrame(item)->bounds;
    BOUNDS_16 fixed_bounds = { 0 };

    // Bounds need to change in order to account for 2 sector trapdoors
    // and the trapdoor angle.
    if (item->rot.y == 0) {
        fixed_bounds.min_x = orig_bounds->min_x;
        fixed_bounds.max_x = orig_bounds->max_x;
        fixed_bounds.min_z = orig_bounds->min_z;
        fixed_bounds.max_z = orig_bounds->max_z;
    } else if (item->rot.y == PHD_90) {
        fixed_bounds.min_x = orig_bounds->min_z;
        fixed_bounds.max_x = orig_bounds->max_z;
        fixed_bounds.min_z = -orig_bounds->max_x;
        fixed_bounds.max_z = -orig_bounds->min_x;
    } else if (item->rot.y == -PHD_180) {
        fixed_bounds.min_x = -orig_bounds->max_x;
        fixed_bounds.max_x = -orig_bounds->min_x;
        fixed_bounds.min_z = -orig_bounds->max_z;
        fixed_bounds.max_z = -orig_bounds->min_z;
    } else if (item->rot.y == -PHD_90) {
        fixed_bounds.min_x = -orig_bounds->max_z;
        fixed_bounds.max_x = -orig_bounds->min_z;
        fixed_bounds.min_z = orig_bounds->min_x;
        fixed_bounds.max_z = orig_bounds->max_x;
    }

    if (x <= item->pos.x + fixed_bounds.max_x
        && x >= item->pos.x + fixed_bounds.min_x
        && z <= item->pos.z + fixed_bounds.max_z
        && z >= item->pos.z + fixed_bounds.min_z) {
        return true;
    }

    return false;
}

void __cdecl Trapdoor_Floor(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    if (!Trapdoor_IsItemOnTop(item, x, z)) {
        return;
    } else if (item->current_anim_state != TRAPDOOR_STATE_CLOSED) {
        return;
    } else if (y > item->pos.y || item->pos.y > *out_height) {
        return;
    }
    *out_height = item->pos.y;
}

void __cdecl Trapdoor_Ceiling(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    if (!Trapdoor_IsItemOnTop(item, x, z)) {
        return;
    } else if (item->current_anim_state != TRAPDOOR_STATE_CLOSED) {
        return;
    } else if (y <= item->pos.y || item->pos.y <= *out_height) {
        return;
    } else {
        *out_height = item->pos.y + STEP_L;
    }
}

void __cdecl Trapdoor_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (Item_IsTriggerActive(item)) {
        if (item->current_anim_state == TRAPDOOR_STATE_CLOSED) {
            item->goal_anim_state = TRAPDOOR_STATE_OPEN;
        }
    } else {
        if (item->current_anim_state == TRAPDOOR_STATE_OPEN) {
            item->goal_anim_state = TRAPDOOR_STATE_CLOSED;
        }
    }
    Item_Animate(item);
}
