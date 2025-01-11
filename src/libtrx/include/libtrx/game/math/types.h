#pragma once

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    int32_t x;
    int32_t z;
} XZ_32;

typedef struct {
    int16_t x;
    int16_t z;
} XZ_16;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} XYZ_32;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} XYZ_16;

typedef enum {
    DIR_UNKNOWN = -1,
    DIR_NORTH = 0,
    DIR_EAST = 1,
    DIR_SOUTH = 2,
    DIR_WEST = 3,
} DIRECTION;

typedef struct {
    XYZ_16 min;
    XYZ_16 max;
} BOUNDS_16;

#if TR_VERSION == 1
typedef struct {
    XYZ_32 min;
    XYZ_32 max;
} BOUNDS_32;
#endif

#pragma pack(pop)
