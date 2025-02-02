#include "game/objects/general/bridge_tilt_2.h"

#include "game/objects/general/bridge_common.h"

void BridgeTilt2_Setup(void)
{
    OBJECT *const obj = Object_Get(O_BRIDGE_TILT_2);
    obj->ceiling = BridgeTilt2_Ceiling;
    obj->floor = BridgeTilt2_Floor;
}

void BridgeTilt2_Floor(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    const int32_t offset_height =
        item->pos.y + (Bridge_GetOffset(item, x, z) / 2);

    if (y > offset_height) {
        return;
    }

    *out_height = offset_height;
}

void BridgeTilt2_Ceiling(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    const int32_t offset_height =
        item->pos.y + (Bridge_GetOffset(item, x, z) / 2);
    if (y <= offset_height) {
        return;
    }

    *out_height = offset_height + STEP_L;
}
