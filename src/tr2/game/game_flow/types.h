#pragma once

#include "global/types.h"

#include <libtrx/game/game_flow/types.h>

// Concrete events data

typedef struct {
    char *path;
    float display_time;
    float fade_in_time;
    float fade_out_time;
} GF_DISPLAY_PICTURE_DATA;

typedef enum {
    GF_INV_REGULAR,
    GF_INV_SECRET,
} GF_INV_TYPE;

typedef struct {
    GAME_OBJECT_ID object_id;
    GF_INV_TYPE inv_type;
    int32_t qty;
} GF_ADD_ITEM_DATA;
