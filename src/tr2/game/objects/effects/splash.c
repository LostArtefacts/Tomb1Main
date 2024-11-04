#include "game/objects/effects/splash.h"

#include "global/funcs.h"

void Splash_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SPLASH);
    obj->control = Splash_Control;
    obj->semi_transparent = 1;
}
