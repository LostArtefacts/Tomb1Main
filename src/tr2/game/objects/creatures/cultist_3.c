#include "game/objects/creatures/cultist_3.h"

#include "game/creature.h"
#include "game/objects/creatures/cultist_common.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

// clang-format off
#define CULTIST_3_HITPOINTS   150
#define CULTIST_3_SHOT_DAMAGE 50
#define CULTIST_3_WALK_TURN   (PHD_DEGREE * 3) // = 546
#define CULTIST_3_RUN_TURN    (PHD_DEGREE * 3) // = 546
#define CULTIST_3_STOP_RANGE  SQUARE(WALL_L * 3) // = 9437184
#define CULTIST_3_RUN_RANGE   SQUARE(WALL_L * 5) // = 26214400
// clang-format on

typedef enum {
    // clang-format off
    CULTIST_3_STATE_EMPTY   = 0,
    CULTIST_3_STATE_STOP    = 1,
    CULTIST_3_STATE_WAIT    = 2,
    CULTIST_3_STATE_WALK    = 3,
    CULTIST_3_STATE_RUN     = 4,
    CULTIST_3_STATE_AIM_L   = 5,
    CULTIST_3_STATE_AIM_R   = 6,
    CULTIST_3_STATE_SHOOT_L = 7,
    CULTIST_3_STATE_SHOOT_R = 8,
    CULTIST_3_STATE_AIM_2   = 9,
    CULTIST_3_STATE_SHOOT_2 = 10,
    CULTIST_3_STATE_DEATH   = 11,
    // clang-format on
} CULTIST_3_STATE;

typedef enum {
    // clang-format off
    CULTIST_3_ANIM_WAIT  = 3,
    CULTIST_3_ANIM_DEATH = 32,
    // clang-format on
} CULTIST_3_ANIM;

static const BITE m_Cultist3LeftGun = {
    .pos = { .x = -2, .y = 275, .z = 23 },
    .mesh_num = 6,
};

static const BITE m_Cultist3RightGun = {
    .pos = { .x = 2, .y = 275, .z = 23 },
    .mesh_num = 10,
};

void Cultist3_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_CULT_3];
    if (!obj->loaded) {
        return;
    }

    obj->initialise = Cultist3_Initialise;
    obj->control = Cultist3_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = CULTIST_3_HITPOINTS;
    obj->radius = CULTIST_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 0;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

void __cdecl Cultist3_Initialise(const int16_t item_num)
{
    Creature_Initialise(item_num);
    ITEM *const item = Item_Get(item_num);
    item->anim_num = g_Objects[O_CULT_3].anim_idx + CULTIST_3_ANIM_WAIT;
    item->frame_num = g_Anims[item->anim_num].frame_base;
    item->goal_anim_state = CULTIST_3_STATE_WAIT;
    item->current_anim_state = CULTIST_3_STATE_WAIT;
}
