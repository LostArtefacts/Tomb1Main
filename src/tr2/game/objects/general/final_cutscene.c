#include "game/objects/general/final_cutscene.h"

#include "global/funcs.h"

void FinalCutscene_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_CUT_SHOTGUN);
    obj->control = FinalCutscene_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
