#include "global/types.h"

#include <libtrx/enum_map.h>
#include <libtrx/game/gamebuf.h>
#include <libtrx/game/input.h>
#include <libtrx/game/objects/ids.h>
#include <libtrx/screenshot.h>

void EnumMap_Init(void)
{
#include "global/enum_map.def"

#include <libtrx/game/enum_map.def>

#undef OBJ_ID_DEFINE
#define OBJ_ID_DEFINE(object_id, value)                                        \
    EnumMap_Define("GAME_OBJECT_ID", object_id, #object_id);
#include <libtrx/game/objects/ids.def>

#undef INPUT_ROLE_DEFINE
#define INPUT_ROLE_DEFINE(role_name, state_name)                               \
    EnumMap_Define("INPUT_ROLE", INPUT_ROLE_##role_name, #state_name);
#include <libtrx/game/input/roles.def>
}
