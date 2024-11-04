#include "game/objects/general/bird_tweeter.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void BirdTweeter_Setup(OBJECT *const obj)
{
    obj->control = BirdTweeter_Control;
    obj->draw_routine = Object_DrawDummyItem;
}
