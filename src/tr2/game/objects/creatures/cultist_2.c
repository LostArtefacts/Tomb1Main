#include "game/objects/creatures/cultist_2.h"

#include "game/creature.h"
#include "game/objects/creatures/cultist_common.h"
#include "global/funcs.h"
#include "global/vars.h"

#define CULTIST_2_HITPOINTS 60

void Cultist2_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_CULT_2];
    if (!obj->loaded) {
        return;
    }

    obj->control = Cultist2_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = CULTIST_2_HITPOINTS;
    obj->radius = CULTIST_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 50;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 8 * 4] |= BF_ROT_Y;
}
