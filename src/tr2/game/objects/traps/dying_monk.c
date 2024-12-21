#include "game/objects/traps/dying_monk.h"

#include "game/gamebuf.h"
#include "game/items.h"
#include "game/room.h"

#define MAX_ROOMIES 2

void __cdecl DyingMonk_Initialise(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    int32_t *const roomies =
        GameBuf_Alloc(sizeof(int32_t) * MAX_ROOMIES, GBUF_TEMP_ALLOC);
    for (int32_t i = 0; i < MAX_ROOMIES; i++) {
        roomies[i] = NO_ITEM;
    }

    int32_t roomie_count = 0;
    int16_t test_item_num = Room_Get(item->room_num)->item_num;
    while (test_item_num != NO_ITEM) {
        const ITEM *const test_item = Item_Get(test_item_num);
        const OBJECT *const test_obj = Object_GetObject(test_item->object_id);
        if (test_obj->intelligent) {
            roomies[roomie_count++] = test_item_num;
            if (roomie_count >= MAX_ROOMIES) {
                break;
            }
        }
        test_item_num = test_item->next_item;
    }

    item->data = roomies;
}
