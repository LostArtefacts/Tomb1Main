#include "game/objects/creatures/worker_3.h"

#include "game/creature.h"
#include "game/objects/common.h"
#include "game/objects/creatures/worker_common.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#define WORKER_3_HITPOINTS 27
#define WORKER_4_HITPOINTS 27

void Worker3_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_WORKER_3];
    if (!obj->loaded) {
        return;
    }

    obj->control = Worker3_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = WORKER_3_HITPOINTS;
    obj->radius = WORKER_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 0;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 4 * 4] |= BF_ROT_Y;
}

void Worker4_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_WORKER_4];
    if (!obj->loaded) {
        return;
    }

    obj->control = Worker3_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = WORKER_4_HITPOINTS;
    obj->radius = WORKER_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 0;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 4 * 4] |= BF_ROT_Y;
}
