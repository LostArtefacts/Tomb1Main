#pragma once

#include <stdbool.h>
#include <stdint.h>

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
