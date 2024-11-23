#pragma once

#include <stdint.h>

#if TR_VERSION == 1
    #define __PACKING
#elif TR_VERSION == 2
    #define __PACKING __attribute__((packed))
#endif

#if TR_VERSION == 1
typedef struct __PACKING {
    uint16_t texture;
    uint16_t vertices[4];
} FACE4;

typedef struct __PACKING {
    uint16_t texture;
    uint16_t vertices[3];
} FACE3;
#endif
