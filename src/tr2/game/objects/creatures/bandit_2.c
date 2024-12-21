#include "game/objects/creatures/bandit_2.h"

#include "game/creature.h"
#include "game/objects/common.h"
#include "game/objects/creatures/bandit_common.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/debug.h>

#define BANDIT_2_HITPOINTS 50

void Bandit2_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BANDIT_2);
    if (!obj->loaded) {
        return;
    }

    obj->control = Bandit2_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = BANDIT_2_HITPOINTS;
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

void Bandit2B_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BANDIT_2B);
    if (!obj->loaded) {
        return;
    }

    const OBJECT *const ref_obj = Object_GetObject(O_BANDIT_2);
    ASSERT(ref_obj->loaded);
    obj->anim_idx = ref_obj->anim_idx;
    obj->frame_base = ref_obj->frame_base;

    obj->control = Bandit2_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = BANDIT_2_HITPOINTS;
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
