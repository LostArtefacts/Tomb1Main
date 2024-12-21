#include "game/objects/traps/spikes.h"

#include "global/funcs.h"

void Spikes_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SPIKES);
    obj->collision = Spikes_Collision;
}
