#include "game/objects/general/ding_dong.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void DingDong_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_DING_DONG);
    obj->control = DingDong_Control;
    obj->draw_routine = Object_DrawDummyItem;
}
