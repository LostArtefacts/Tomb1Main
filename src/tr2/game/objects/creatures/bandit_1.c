#include "game/objects/creatures/bandit_1.h"

#include "game/creature.h"
#include "game/objects/common.h"
#include "game/objects/creatures/bandit_common.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/debug.h>

#define BANDIT_1_HITPOINTS 45

void Bandit1_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BANDIT_1);
    if (!obj->loaded) {
        return;
    }

    obj->control = Bandit1_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = BANDIT_1_HITPOINTS;
    obj->radius = BANDIT_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 0;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx + 6 * 4] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 8 * 4] |= BF_ROT_Y;
}
