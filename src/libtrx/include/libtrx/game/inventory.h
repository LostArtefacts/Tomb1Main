#pragma once

#include "objects/ids.h"

#include <stdbool.h>
#include <stdint.h>

extern bool Inv_AddItem(GAME_OBJECT_ID object_id);
extern bool Inv_AddItemNTimes(GAME_OBJECT_ID object_id, int32_t qty);
extern int32_t Inv_RequestItem(const GAME_OBJECT_ID object_id);
