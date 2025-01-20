#pragma once

#include <stdint.h>

typedef struct {
    uint16_t u;
    uint16_t v;
} PHD_UV;

typedef struct {
    uint16_t draw_type;
    uint16_t tex_page;
    PHD_UV uv[4];
#if TR_VERSION == 2
    PHD_UV uv_backup[4];
#endif
} OBJECT_TEXTURE;

typedef struct {
    uint16_t tex_page;
    uint16_t offset;
    uint16_t width;
    uint16_t height;
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
} SPRITE_TEXTURE;
