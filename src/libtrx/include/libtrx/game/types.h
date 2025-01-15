#pragma once

#include "./items/types.h"
#include "./math.h"

#include <stdint.h>

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        XYZ_32 pos;
    };
    int16_t room_num;
} GAME_VECTOR;

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        XYZ_32 pos;
    };
    int16_t data;
    int16_t flags;
} OBJECT_VECTOR;

typedef struct {
    uint16_t texture;
    uint16_t vertices[4];
    bool enable_reflections;
} FACE4;

typedef struct {
    uint16_t texture;
    uint16_t vertices[3];
    bool enable_reflections;
} FACE3;
