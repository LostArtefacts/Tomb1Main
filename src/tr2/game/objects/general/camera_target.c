#include "game/objects/general/camera_target.h"

#include "game/objects/common.h"

void CameraTarget_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_CAMERA_TARGET);
    obj->draw_routine = Object_DrawDummyItem;
}
