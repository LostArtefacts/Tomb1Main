#pragma once

#include "./types.h"

void GF_InventoryModifier_Reset(void);
void GF_InventoryModifier_Add(GAME_OBJECT_ID object_id, GF_INV_TYPE type);
void GF_InventoryModifier_Apply(int32_t level, GF_INV_TYPE type);
