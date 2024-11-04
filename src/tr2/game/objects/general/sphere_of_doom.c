#include "game/objects/general/sphere_of_doom.h"

#include "global/funcs.h"

void SphereOfDoom_Setup(OBJECT *const obj, const bool transparent)
{
    obj->collision = SphereOfDoom_Collision;
    obj->control = SphereOfDoom_Control;
    obj->draw_routine = SphereOfDoom_Draw;
    obj->save_position = 1;
    obj->save_flags = 1;
    if (transparent) {
        obj->semi_transparent = 1;
    }
}
