#include "game/objects/traps/teeth_trap.h"

#include "game/items.h"
#include "game/lara/control.h"
#include "game/objects/common.h"
#include "global/funcs.h"

#define TEETH_TRAP_DAMAGE 400

typedef enum {
    TEETH_TRAP_STATE_NICE = 0,
    TEETH_TRAP_STATE_NASTY = 1,
} TEETH_TRAP_STATE;

static const BITE m_Teeth[6] = {
    // clang-format off
    { .pos = { .x = -23, .y = 0,   .z = -1718 }, .mesh_num = 0 },
    { .pos = { .x = 71,  .y = 0,   .z = -1718 }, .mesh_num = 1 },
    { .pos = { .x = -23, .y = 10,  .z = -1718 }, .mesh_num = 0 },
    { .pos = { .x = 71,  .y = 10,  .z = -1718 }, .mesh_num = 1 },
    { .pos = { .x = -23, .y = -10, .z = -1718 }, .mesh_num = 0 },
    { .pos = { .x = 71,  .y = -10, .z = -1718 }, .mesh_num = 1 },
    // clang-format on
};

void __cdecl TeethTrap_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    if (Item_IsTriggerActive(item)) {
        item->goal_anim_state = TEETH_TRAP_STATE_NASTY;
        if (item->touch_bits != 0
            && item->current_anim_state == TEETH_TRAP_STATE_NASTY) {
            Lara_TakeDamage(TEETH_TRAP_DAMAGE, true);
            BaddieBiteEffect(item, &m_Teeth[0]);
            BaddieBiteEffect(item, &m_Teeth[1]);
            BaddieBiteEffect(item, &m_Teeth[2]);
            BaddieBiteEffect(item, &m_Teeth[3]);
            BaddieBiteEffect(item, &m_Teeth[4]);
            BaddieBiteEffect(item, &m_Teeth[5]);
        }
    } else {
        item->goal_anim_state = TEETH_TRAP_STATE_NICE;
    }

    Item_Animate(item);
}

void TeethTrap_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_TEETH_TRAP);
    obj->control = TeethTrap_Control;
    obj->collision = Object_Collision_Trap;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
