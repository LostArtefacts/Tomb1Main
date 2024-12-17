#include "game/objects/creatures/bird_guardian.h"

#include "game/creature.h"
#include "game/objects/common.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#define BIRD_GUARDIAN_HITPOINTS 200
#define BIRD_GUARDIAN_RADIUS (WALL_L / 3) // = 341

void BirdGuardian_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_BIRD_GUARDIAN];
    if (!obj->loaded) {
        return;
    }

    obj->control = BirdGuardian_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = BIRD_GUARDIAN_HITPOINTS;
    obj->radius = BIRD_GUARDIAN_RADIUS;

    obj->intelligent = 1;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
}
