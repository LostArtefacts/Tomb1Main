#pragma once

#include "./types.h"

void GF_InventoryModifier_Reset(void);
void GF_InventoryModifier_Add(GF_ADD_ITEM item_type, GF_INV_TYPE type);
void GF_InventoryModifier_Apply(int32_t level, GF_INV_TYPE type);
