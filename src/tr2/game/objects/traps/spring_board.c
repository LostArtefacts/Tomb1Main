#include "game/objects/traps/spring_board.h"

#include "game/objects/common.h"
#include "global/funcs.h"

void SpringBoard_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_SPRING_BOARD);
    obj->control = SpringBoard_Control;
    obj->save_flags = 1;
    obj->save_anim = 1;
}
