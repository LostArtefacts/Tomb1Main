#include "game/objects/traps/dart.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void Dart_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DART);
    obj->control = Dart_Control;
    obj->collision = Object_Collision;
    obj->shadow_size = 128;
}
