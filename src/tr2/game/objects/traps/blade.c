#include "game/objects/traps/blade.h"

#include "game/objects/common.h"
#include "global/funcs.h"
#include "global/vars.h"

typedef enum {
    // clang-format off
    BLADE_STATE_EMPTY = 0,
    BLADE_STATE_STOP  = 1,
    BLADE_STATE_CUT   = 2,
    // clang-format on
} BLADE_STATE;

typedef enum {
    // clang-format off
    BLADE_ANIM_RETURN   = 0,
    BLADE_ANIM_FINISHED = 1,
    BLADE_ANIM_SET      = 2,
    BLADE_ANIM_CUT      = 3,
    // clang-format on
} BLADE_ANIM;

void __cdecl Blade_Initialise(const int16_t item_num)
{
    const OBJECT *const obj = Object_GetObject(O_BLADE);
    ITEM *const item = Item_Get(item_num);
    item->anim_num = obj->anim_idx + BLADE_ANIM_SET;
    item->frame_num = g_Anims[item->anim_num].frame_base;
    item->current_anim_state = BLADE_STATE_STOP;
}

void Blade_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BLADE);
    obj->initialise = Blade_Initialise;
    obj->control = Blade_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
