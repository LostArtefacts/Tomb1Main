#include "game/objects/general/bridge_tilt_1.h"

#include "game/objects/general/bridge_common.h"

void BridgeTilt1_Setup(void)
{
    OBJECT *const obj = Object_Get(O_BRIDGE_TILT_1);
    obj->ceiling = BridgeTilt1_Ceiling;
    obj->floor = BridgeTilt1_Floor;
}

void BridgeTilt1_Floor(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    const int32_t offset_height =
        item->pos.y + (Bridge_GetOffset(item, x, z) / 4);

    if (y > offset_height) {
        return;
    }

    *out_height = offset_height;
}

void BridgeTilt1_Ceiling(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    const int32_t offset_height =
        item->pos.y + (Bridge_GetOffset(item, x, z) / 4);

    if (y <= offset_height) {
        return;
    }

    *out_height = offset_height + STEP_L;
}
