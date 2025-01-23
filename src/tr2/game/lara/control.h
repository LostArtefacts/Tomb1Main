#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <libtrx/game/lara/common.h>

void Lara_HandleAboveWater(ITEM *item, COLL_INFO *coll);

void Lara_HandleSurface(ITEM *item, COLL_INFO *coll);

void Lara_HandleUnderwater(ITEM *item, COLL_INFO *coll);

void Lara_Control(int16_t item_num);
void Lara_ControlExtra(int16_t item_num);

void Lara_UseItem(GAME_OBJECT_ID object_id);

void Lara_InitialiseLoad(int16_t item_num);

void Lara_Initialise(const GAME_FLOW_LEVEL *level);

void Lara_InitialiseInventory(const GAME_FLOW_LEVEL *level);

void Lara_InitialiseMeshes(const GAME_FLOW_LEVEL *level);

void Lara_GetOffVehicle(void);

void Lara_TakeDamage(int16_t damage, bool hit_status);
