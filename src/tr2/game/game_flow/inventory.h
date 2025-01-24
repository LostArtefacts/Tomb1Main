#pragma once

#include "./types.h"

void GF_InventoryModifier_Reset(void);
void GF_InventoryModifier_Add(
    GAME_OBJECT_ID object_id, GAME_FLOW_INV_TYPE type, int32_t qty);
void GF_InventoryModifier_Apply(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_INV_TYPE type);
