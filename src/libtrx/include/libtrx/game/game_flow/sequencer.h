#pragma once

#include "../inventory_ring/types.h"
#include "./types.h"

GF_COMMAND GF_EnterPhotoMode(void);
GF_COMMAND GF_PauseGame(void);
GF_COMMAND GF_ShowInventory(INVENTORY_MODE inv_mode);
GF_COMMAND GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);
GF_COMMAND GF_RunDemo(int32_t demo_num);
GF_COMMAND GF_RunCutscene(int32_t cutscene_num);
GF_COMMAND GF_RunGame(const GF_LEVEL *level, GF_SEQUENCE_CONTEXT seq_ctx);
