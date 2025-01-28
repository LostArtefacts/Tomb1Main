#pragma once

#include <stdint.h>

typedef enum {
    BK_TRANSPARENT,
    BK_OBJECT,
    BK_IMAGE,
} BACKGROUND_TYPE;

typedef struct {
    int16_t value_1;
    int16_t value_2;
} SHADE;

typedef struct {
    int32_t value_1;
    int32_t value_2;
} FALLOFF;

typedef struct {
    uint16_t u;
    uint16_t v;
} TEXTURE_UV;

typedef struct {
    uint16_t draw_type;
    uint16_t tex_page;
    TEXTURE_UV uv[4];
#if TR_VERSION == 2
    TEXTURE_UV uv_backup[4];
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

typedef struct ANIMATED_TEXTURE_RANGE {
    int16_t num_textures;
    int16_t *textures;
    struct ANIMATED_TEXTURE_RANGE *next_range;
} ANIMATED_TEXTURE_RANGE;

typedef struct {
    float r;
    float g;
    float b;
} RGB_F;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_888;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA_8888;
