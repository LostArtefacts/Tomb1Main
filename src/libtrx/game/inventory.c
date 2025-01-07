#include "game/inventory.h"

#include "game/inventory_ring/vars.h"
#include "game/objects/vars.h"

bool Inv_AddItemNTimes(const GAME_OBJECT_ID object_id, const int32_t qty)
{
    bool result = false;
    for (int32_t i = 0; i < qty; i++) {
        result |= Inv_AddItem(object_id);
    }
    return result;
}

GAME_OBJECT_ID Inv_GetItemOption(const GAME_OBJECT_ID object_id)
{
    if (Object_IsObjectType(object_id, g_InvObjects)) {
        return object_id;
    }
    return Object_GetCognate(object_id, g_ItemToInvObjectMap);
}

void Inv_InsertItem(INVENTORY_ITEM *const inv_item)
{
    INV_RING_SOURCE *const source =
        &g_InvRing_Source[inv_item->inv_pos < 100 ? RT_MAIN : RT_KEYS];

    int32_t n;
    for (n = 0; n < source->count; n++) {
        if (source->items[n]->inv_pos > inv_item->inv_pos) {
            break;
        }
    }

    for (int32_t i = source->count; i > n - 1; i--) {
        source->items[i + 1] = source->items[i];
        source->qtys[i + 1] = source->qtys[i];
    }
    source->items[n] = inv_item;
    source->qtys[n] = 1;
    source->count++;
}

bool Inv_RemoveItem(const GAME_OBJECT_ID object_id)
{
    const GAME_OBJECT_ID inv_object_id = Inv_GetItemOption(object_id);
    for (RING_TYPE ring_type = 0; ring_type < RT_NUMBER_OF; ring_type++) {
        INV_RING_SOURCE *const source = &g_InvRing_Source[ring_type];
        for (int32_t i = 0; i < source->count; i++) {
            if (source->items[i]->object_id == inv_object_id) {
                source->qtys[i]--;
                if (source->qtys[i] == 0) {
                    source->count--;
                    for (int32_t j = i; j < source->count; j++) {
                        source->items[j] = source->items[j + 1];
                        source->qtys[j] = source->qtys[j + 1];
                    }
                }
                return true;
            }
        }
    }
    return false;
}

int32_t Inv_RequestItem(const GAME_OBJECT_ID object_id)
{
    const GAME_OBJECT_ID inv_object_id = Inv_GetItemOption(object_id);
    for (RING_TYPE ring_type = 0; ring_type < RT_NUMBER_OF; ring_type++) {
        INV_RING_SOURCE *const source = &g_InvRing_Source[ring_type];
        for (int32_t i = 0; i < source->count; i++) {
            if (source->items[i]->object_id == inv_object_id) {
                return source->qtys[i];
            }
        }
    }
    return 0;
}

void Inv_ClearSelection(void)
{
    g_InvRing_Source[RT_MAIN].current = 0;
    g_InvRing_Source[RT_KEYS].current = 0;
}

void Inv_RemoveAllItems(void)
{
    // leave only the first item, which is the compass / stopwatch
    g_InvRing_Source[RT_MAIN].count = 1;
    g_InvRing_Source[RT_KEYS].count = 0;
    Inv_ClearSelection();
}
