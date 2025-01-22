#pragma once

#include "./types.h"

#include <libtrx/game/inventory_ring/types.h>

GAME_FLOW_COMMAND GF_StartGame(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_EnterPhotoMode(void);
GAME_FLOW_COMMAND GF_PauseGame(void);
GAME_FLOW_COMMAND GF_ShowInventory(INVENTORY_MODE mode);
GAME_FLOW_COMMAND GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);

GAME_FLOW_COMMAND GF_InterpretSequence(
    const GAME_FLOW_SEQUENCE *sequence, GAME_FLOW_LEVEL_TYPE type);

bool GF_DoFrontendSequence(void);
GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num);
GAME_FLOW_COMMAND GF_DoLevelSequence(
    int32_t start_level, GAME_FLOW_LEVEL_TYPE type);
