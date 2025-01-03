#pragma once

#include "global/types.h"

void Lara_HandleAboveWater(ITEM *item, COLL_INFO *coll);

void Lara_HandleSurface(ITEM *item, COLL_INFO *coll);

void Lara_HandleUnderwater(ITEM *item, COLL_INFO *coll);

void Lara_Control(int16_t item_num);
void Lara_ControlExtra(int16_t item_num);

void Lara_Animate(ITEM *item);

void Lara_UseItem(GAME_OBJECT_ID object_id);

void Lara_InitialiseLoad(int16_t item_num);

void Lara_Initialise(GAME_FLOW_LEVEL_TYPE type);

void Lara_InitialiseInventory(int32_t level_num);

void Lara_InitialiseMeshes(int32_t level_num);

void Lara_GetOffVehicle(void);
void Lara_SwapSingleMesh(LARA_MESH mesh, GAME_OBJECT_ID);

int16_t Lara_GetNearestEnemy(void);
void Lara_TakeDamage(int16_t damage, bool hit_status);
