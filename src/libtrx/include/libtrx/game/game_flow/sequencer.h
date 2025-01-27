#pragma once

#include "../inventory_ring/types.h"
#include "./types.h"

GAME_FLOW_COMMAND GF_EnterPhotoMode(void);
GAME_FLOW_COMMAND GF_PauseGame(void);
GAME_FLOW_COMMAND GF_ShowInventory(INVENTORY_MODE inv_mode);
GAME_FLOW_COMMAND GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);
GAME_FLOW_COMMAND GF_RunDemo(int32_t demo_num);
GAME_FLOW_COMMAND GF_RunCutscene(int32_t cutscene_num);
GAME_FLOW_COMMAND GF_RunGame(
    const GAME_FLOW_LEVEL *level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx);
