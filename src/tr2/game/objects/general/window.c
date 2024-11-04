#include "game/objects/general/window.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Window1_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WINDOW_1);
    obj->initialise = Window_Initialise;
    obj->collision = Object_Collision;
    obj->control = Window_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

void Window2_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_WINDOW_2);
    obj->initialise = Window_Initialise;
    obj->collision = Object_Collision;
    obj->control = SmashIce_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
