#pragma once

#include "global/types_decomp.h"

typedef struct {
    uint16_t draw_type;
    uint16_t tex_page;
    PHD_UV uv[4];
    PHD_UV uv_backup[4];
} PHD_TEXTURE;

typedef enum {
    LIGHTING_CONTRAST_LOW,
    LIGHTING_CONTRAST_MEDIUM,
    LIGHTING_CONTRAST_HIGH,
    LIGHTING_CONTRAST_NUMBER_OF,
} LIGHTING_CONTRAST;
