#include "game/objects/traps/killer_statue.h"

#include "game/objects/common.h"
#include "global/funcs.h"
#include "global/vars.h"

typedef enum {
    // clang-format off
    KILLER_STATUE_STATE_EMPTY = 0,
    KILLER_STATUE_STATE_STOP  = 1,
    KILLER_STATUE_STATE_CUT   = 2,
    // clang-format on
} KILLER_STATUE_STATE;

typedef enum {
    // clang-format off
    KILLER_STATUE_ANIM_RETURN   = 0,
    KILLER_STATUE_ANIM_FINISHED = 1,
    KILLER_STATUE_ANIM_CUT      = 2,
    KILLER_STATUE_ANIM_SET      = 3,
    // clang-format on
} KILLER_STATUE_ANIM;

void __cdecl KillerStatue_Initialise(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    const OBJECT *const obj = Object_GetObject(item->object_id);
    item->anim_num = obj->anim_idx + KILLER_STATUE_ANIM_SET;
    item->frame_num = g_Anims[item->anim_num].frame_base;
    item->current_anim_state = KILLER_STATUE_STATE_STOP;
}

void KillerStatue_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_KILLER_STATUE);
    obj->initialise = KillerStatue_Initialise;
    obj->control = KillerStatue_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
