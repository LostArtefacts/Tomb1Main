#include "game/objects/traps/dart_emitter.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void DartEmitter_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DART_EMITTER);
    obj->control = DartEmitter_Control;
    obj->save_flags = 1;
}
