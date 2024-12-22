#include "game/objects/general/clock_chimes.h"

#include "game/objects/common.h"
#include "game/sound.h"
#include "global/funcs.h"

#include <libtrx/game/lara/common.h>

void __cdecl DoChimeSound(const ITEM *const item)
{
    const ITEM *const lara_item = Lara_GetItem();
    XYZ_32 pos = lara_item->pos;
    pos.x += (item->pos.x - lara_item->pos.x) >> 6;
    pos.y += (item->pos.y - lara_item->pos.y) >> 6;
    pos.z += (item->pos.z - lara_item->pos.z) >> 6;
    Sound_Effect(SFX_DOOR_CHIME, &pos, SPM_NORMAL);
}

void ClockChimes_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_CLOCK_CHIMES);
    obj->control = ClockChimes_Control;
    obj->draw_routine = Object_DrawDummyItem;
    obj->save_flags = 1;
}
