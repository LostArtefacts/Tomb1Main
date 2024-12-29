#pragma once

#include "objects/ids.h"

#include <stdbool.h>
#include <stdint.h>

bool Inv_AddItem(GAME_OBJECT_ID object_id);
bool Inv_AddItemNTimes(GAME_OBJECT_ID object_id, int32_t qty);
