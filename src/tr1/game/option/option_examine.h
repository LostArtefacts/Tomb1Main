#pragma once

#include <libtrx/game/objects/types.h>

bool Option_Examine_CanExamine(GAME_OBJECT_ID object_id);
bool Option_Examine_IsActive(void);
void Option_Examine_Control(GAME_OBJECT_ID object_id);
void Option_Examine_Draw(void);
void Option_Examine_Shutdown(void);
