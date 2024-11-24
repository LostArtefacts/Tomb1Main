#pragma once

#include <stdbool.h>
#include <stdint.h>

#if TR_VERSION == 1
    #define __PACKING
#elif TR_VERSION == 2
    #define __PACKING __attribute__((packed))
#endif

typedef struct __PACKING {
    uint16_t texture;
    uint16_t vertices[4];
    bool enable_reflections;
} FACE4;

typedef struct __PACKING {
    uint16_t texture;
    uint16_t vertices[3];
    bool enable_reflections;
} FACE3;
