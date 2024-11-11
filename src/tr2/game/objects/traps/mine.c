#include "game/objects/traps/mine.h"

#include "decomp/effects.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#include <libtrx/utils.h>

static int16_t M_GetBoatItem(const XYZ_32 *const pos, int16_t *room_num);
static void M_DetonateAll(
    const ITEM *mine_item, int16_t boat_item_num, int16_t boat_room_num);
static void M_Explode(ITEM *mine_item);

static int16_t M_GetBoatItem(const XYZ_32 *const pos, int16_t *const room_num)
{
    Room_GetSector(pos->x, pos->y, pos->z, room_num);

    int16_t item_num = g_Rooms[*room_num].item_num;
    while (item_num != NO_ITEM) {
        const ITEM *const item = Item_Get(item_num);
        if (item->object_id == O_BOAT) {
            const int32_t dx = item->pos.x - pos->x;
            const int32_t dz = item->pos.z - pos->z;
            // TODO: fix overflows and no y check
            if (SQUARE(dx) + SQUARE(dz) < SQUARE(WALL_L / 2)) {
                break;
            }
        }
        item_num = item->next_item;
    }
    return item_num;
}

static void M_DetonateAll(
    const ITEM *const mine_item, const int16_t boat_item_num,
    int16_t boat_room_num)
{
    ITEM *const boat_item = Item_Get(boat_item_num);
    if (g_Lara.skidoo == boat_item_num) {
        Effect_ExplodingDeath(g_Lara.item_num, -1, 0);
        g_LaraItem->hit_points = 0;
        g_LaraItem->flags |= IF_ONE_SHOT;
    }

    boat_item->object_id = O_BOAT_BITS;
    Effect_ExplodingDeath(boat_item_num, -1, 0);
    Item_Kill(boat_item_num);
    boat_item->object_id = O_BOAT;

    const SECTOR *const sector = Room_GetSector(
        mine_item->pos.x, mine_item->pos.y, mine_item->pos.z, &boat_room_num);
    Room_GetHeight(
        sector, mine_item->pos.x, mine_item->pos.y, mine_item->pos.z);
    Room_TestTriggers(g_TriggerIndex, true);

    g_DetonateAllMines = true;
}

static void M_Explode(ITEM *const mine_item)
{
    const int16_t fx_num = Effect_Create(mine_item->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->object_id = O_EXPLOSION;
        fx->pos.x = mine_item->pos.x;
        fx->pos.y = mine_item->pos.y - WALL_L;
        fx->pos.z = mine_item->pos.z;
        fx->speed = 0;
        fx->frame_num = 0;
        fx->counter = 0;
    }

    Splash(mine_item);
    Sound_Effect(SFX_EXPLOSION_1, &mine_item->pos, SPM_NORMAL);

    mine_item->flags |= IF_ONE_SHOT;
    mine_item->mesh_bits = 1;
    mine_item->collidable = 0;
}

void __cdecl Mine_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (item->flags & IF_ONE_SHOT) {
        return;
    }

    if (!g_DetonateAllMines) {
        int16_t boat_room_num = item->room_num;
        XYZ_32 test_pos = {
            .x = item->pos.x,
            .y = item->pos.y - WALL_L * 2,
            .z = item->pos.z,
        };
        const int16_t boat_item_num = M_GetBoatItem(&test_pos, &boat_room_num);
        if (boat_item_num == NO_ITEM) {
            return;
        }

        M_DetonateAll(item, boat_item_num, boat_room_num);
    } else if (Random_GetControl() < 0x7800) {
        return;
    }

    M_Explode(item);
}
