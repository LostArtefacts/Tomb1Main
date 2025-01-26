#pragma once

#include "global/types.h"

#include <libtrx/game/game_flow/types.h>

// Concrete events data

typedef struct {
    char *path;
    float display_time;
    float fade_in_time;
    float fade_out_time;
} GAME_FLOW_DISPLAY_PICTURE_DATA;

typedef enum {
    GF_INV_REGULAR,
    GF_INV_SECRET,
} GAME_FLOW_INV_TYPE;

typedef struct {
    GAME_OBJECT_ID object_id;
    GAME_FLOW_INV_TYPE inv_type;
    int32_t qty;
} GAME_FLOW_ADD_ITEM_DATA;

// ----------------------------------------------------------------------------
// Game information
// ----------------------------------------------------------------------------

typedef struct {
    struct {
        GAME_FLOW_LEVEL_TYPE type;
        int32_t num;
    } current_level;
} GAME_INFO;
