#include "game/objects/traps/dart.h"

#include "decomp/effects.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/objects/common.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#include <libtrx/game/lara/common.h>

#define DART_DAMAGE 50

static void M_Hit(int16_t item_num);

static void M_Hit(const int16_t item_num)
{
    const ITEM *const item = Item_Get(item_num);
    Item_Kill(item_num);
    Sound_Effect(SFX_CIRCLE_BLADE_HIT, &item->pos, SPM_NORMAL);

    const int16_t fx_num = Effect_Create(item->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->object_id = O_RICOCHET;
        fx->pos = item->pos;
        fx->rot = item->rot;
        fx->room_num = item->room_num;
        fx->speed = 0;
        fx->counter = 6;
        fx->frame_num = -3 * Random_GetControl() / 0x8000;
    }
}

void __cdecl Dart_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (item->touch_bits != 0) {
        Lara_TakeDamage(DART_DAMAGE, true);
        const ITEM *const lara_item = Lara_GetItem();
        DoBloodSplat(
            item->pos.x, item->pos.y, item->pos.z, lara_item->speed,
            lara_item->rot.y, lara_item->room_num);
    }

    Item_Animate(item);

    int16_t room_num = item->room_num;
    const SECTOR *const sector =
        Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);
    if (item->room_num != room_num) {
        Item_NewRoom(item_num, room_num);
    }
    const int32_t height =
        Room_GetHeight(sector, item->pos.x, item->pos.y, item->pos.z);

    item->rot.x += 4096;
    item->floor = height;
    if (item->pos.y >= height) {
        M_Hit(item_num);
    }
}

void Dart_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DART);
    obj->control = Dart_Control;
    obj->collision = Object_Collision;
    obj->shadow_size = 128;
}
