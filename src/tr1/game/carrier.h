#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

void Carrier_InitialiseLevel(const GAME_FLOW_LEVEL *level);
int32_t Carrier_GetItemCount(int16_t item_num);
bool Carrier_IsItemCarried(int16_t item_num);
void Carrier_TestItemDrops(int16_t item_num);
void Carrier_TestLegacyDrops(int16_t item_num);
DROP_STATUS Carrier_GetSaveStatus(const CARRIED_ITEM *item);
void Carrier_AnimateDrops(void);
