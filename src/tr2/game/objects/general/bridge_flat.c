#include "game/objects/general/bridge_flat.h"

void BridgeFlat_Setup(void)
{
    OBJECT *const obj = Object_Get(O_BRIDGE_FLAT);
    obj->ceiling = BridgeFlat_Ceiling;
    obj->floor = BridgeFlat_Floor;
}

void BridgeFlat_Floor(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    if (y <= item->pos.y) {
        *out_height = item->pos.y;
    }
}

void BridgeFlat_Ceiling(
    const ITEM *const item, const int32_t x, const int32_t y, const int32_t z,
    int32_t *const out_height)
{
    if (y > item->pos.y) {
        *out_height = item->pos.y + STEP_L;
    }
}
