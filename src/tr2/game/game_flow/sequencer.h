#pragma once

#include <libtrx/game/game_flow/types.h>
#include <libtrx/game/inventory_ring/types.h>

GAME_FLOW_COMMAND GF_StartDemo(int32_t level_num);
GAME_FLOW_COMMAND GF_StartGame(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_EnterPhotoMode(void);
GAME_FLOW_COMMAND GF_PauseGame(void);
GAME_FLOW_COMMAND GF_ShowInventory(INVENTORY_MODE mode);
GAME_FLOW_COMMAND GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);
