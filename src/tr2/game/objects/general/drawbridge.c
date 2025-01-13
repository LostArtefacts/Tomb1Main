#include "game/objects/general/drawbridge.h"

#include "game/objects/general/door.h"
#include "game/objects/general/general.h"

typedef enum {
    DRAWBRIDGE_STATE_CLOSED = DOOR_STATE_CLOSED,
    DRAWBRIDGE_STATE_OPEN = DOOR_STATE_OPEN,
} DRAWBRIDGE_STATE;

void Drawbridge_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DRAWBRIDGE);
    if (!obj->loaded) {
        return;
    }
    obj->control = General_Control;
    obj->collision = Drawbridge_Collision;
    obj->ceiling = Drawbridge_Ceiling;
    obj->floor = Drawbridge_Floor;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

int32_t Drawbridge_IsItemOnTop(
    const ITEM *const item, const int32_t z, const int32_t x)
{
    // drawbridge sector
    const XZ_32 obj = {
        .x = item->pos.x >> WALL_SHIFT,
        .z = item->pos.z >> WALL_SHIFT,
    };

    // test sector
    const XZ_32 test = {
        .x = x >> WALL_SHIFT,
        .z = z >> WALL_SHIFT,
    };

    switch (item->rot.y) {
    case 0:
        return test.x == obj.x && (test.z == obj.z - 1 || test.z == obj.z - 2);

    case -DEG_180:
        return test.x == obj.x && (test.z == obj.z + 1 || test.z == obj.z + 2);

    case -DEG_90:
        return test.z == obj.z && (test.x == obj.x + 1 || test.x == obj.x + 2);

    case DEG_90:
        return test.z == obj.z && (test.x == obj.x - 1 || test.x == obj.x - 2);
    }

    return false;
}

void Drawbridge_Floor(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    if (item->current_anim_state != DRAWBRIDGE_STATE_OPEN) {
        return;
    } else if (!Drawbridge_IsItemOnTop(item, z, x)) {
        return;
    } else if (item->pos.y < y) {
        return;
    }
    *out_height = item->pos.y;
}

void Drawbridge_Ceiling(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    if (item->current_anim_state != DRAWBRIDGE_STATE_OPEN) {
        return;
    } else if (!Drawbridge_IsItemOnTop(item, z, x)) {
        return;
    } else if (item->pos.y >= y) {
        return;
    }
    *out_height = item->pos.y + STEP_L;
}

void Drawbridge_Collision(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    const ITEM *const item = Item_Get(item_num);
    if (item->current_anim_state == DRAWBRIDGE_STATE_CLOSED) {
        Door_Collision(item_num, lara_item, coll);
    }
}
