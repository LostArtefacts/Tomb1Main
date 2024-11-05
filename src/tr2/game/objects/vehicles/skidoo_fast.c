#include "game/objects/vehicles/skidoo_fast.h"

#include "decomp/skidoo.h"

void SkidooFast_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SKIDOO_FAST);
    obj->initialise = Skidoo_Initialise;
    obj->draw_routine = Skidoo_Draw;
    obj->collision = Skidoo_Collision;
    obj->save_position = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
