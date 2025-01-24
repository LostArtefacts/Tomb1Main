#pragma once

#include "global/types.h"

#include <libtrx/game/game_flow/types.h>
#include <libtrx/game/objects/types.h>

// ----------------------------------------------------------------------------
// Sequencer structures
// ----------------------------------------------------------------------------

// Concrete events data

typedef struct {
    char *path;
    double display_time;
} GAME_FLOW_DISPLAY_PICTURE_DATA;

typedef struct {
    GAME_OBJECT_ID object1_id;
    GAME_OBJECT_ID object2_id;
    int32_t mesh_num;
} GAME_FLOW_MESH_SWAP_DATA;

typedef struct {
    GAME_OBJECT_ID object_id;
    int32_t quantity;
} GAME_FLOW_ADD_ITEM_DATA;
