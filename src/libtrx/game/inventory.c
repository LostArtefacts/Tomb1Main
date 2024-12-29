#include "game/inventory.h"

bool Inv_AddItemNTimes(const GAME_OBJECT_ID object_id, const int32_t qty)
{
    bool result = false;
    for (int32_t i = 0; i < qty; i++) {
        result |= Inv_AddItem(object_id);
    }
    return result;
}
