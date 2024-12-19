#include "game/objects/creatures/cultist_3.h"

#include "game/creature.h"
#include "game/objects/creatures/cultist_common.h"
#include "global/funcs.h"
#include "global/vars.h"

#define CULTIST_3_HITPOINTS 150

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
