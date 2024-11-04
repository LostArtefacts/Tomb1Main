#include "game/objects/effects/body_part.h"

#include "global/funcs.h"

void BodyPart_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BODY_PART);
    obj->control = BodyPart_Control;
    obj->loaded = 1;
    obj->mesh_count = 0;
}
