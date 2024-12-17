#pragma once

#include "../math.h"
#include "./enum.h"

#include <stdbool.h>

typedef struct __PACKING {
    TRIGGER_OBJECT type;
    void *parameter;
} TRIGGER_CMD;

typedef struct __PACKING {
    int16_t camera_num;
    uint8_t timer;
    uint8_t glide;
    bool one_shot;
} TRIGGER_CAMERA_DATA;

typedef struct __PACKING {
    TRIGGER_TYPE type;
    int8_t timer;
    int16_t mask;
    bool one_shot;
    int16_t item_index;
    int32_t command_count;
    TRIGGER_CMD *commands;
} TRIGGER;

typedef struct __PACKING {
    int16_t room_num;
    XYZ_16 normal;
    XYZ_16 vertex[4];
} PORTAL;

typedef struct __PACKING {
    uint16_t count;
    PORTAL portal[];
} PORTALS;

typedef struct __PACKING {
    uint16_t idx;
    int16_t box;
    bool is_death_sector;
#if TR_VERSION == 2
    LADDER_DIRECTION ladder;
#endif
    TRIGGER *trigger;
    struct __PACKING {
        uint8_t pit;
        uint8_t sky;
        int16_t wall;
    } portal_room;
    struct __PACKING {
        int16_t height;
        int16_t tilt;
    } floor, ceiling;
} SECTOR;

typedef struct __PACKING {
    XYZ_32 pos;
#if TR_VERSION == 1
    int16_t intensity;
    int32_t falloff;
#elif TR_VERSION == 2
    int16_t intensity_1;
    int16_t intensity_2;
    int32_t falloff_1;
    int32_t falloff_2;
#endif
} LIGHT;

#if TR_VERSION == 1
typedef struct __PACKING {
    XYZ_16 pos;
    uint16_t flags;
    uint16_t shade;
} ROOM_VERTEX;

typedef struct __PACKING {
    uint16_t texture;
    uint16_t vertex;
} ROOM_SPRITE;

typedef struct __PACKING {
    int16_t num_vertices;
    int16_t num_face4s;
    int16_t num_face3s;
    int16_t num_sprites;
    ROOM_VERTEX *vertices;
    FACE4 *face4s;
    FACE3 *face3s;
    ROOM_SPRITE *sprites;
} ROOM_MESH;
#elif TR_VERSION == 2
typedef struct __PACKING {
    XYZ_16 pos;
    int16_t light_base;
    uint8_t light_table_value;
    uint8_t flags;
    int16_t light_adder;
} ROOM_VERTEX;
#endif

typedef struct __PACKING {
    XYZ_32 pos;
    struct __PACKING {
        int16_t y;
    } rot;
#if TR_VERSION == 1
    uint16_t shade;
#elif TR_VERSION == 2
    int16_t shade_1;
    int16_t shade_2;
#endif
    int16_t static_num;
} STATIC_MESH;

typedef struct __PACKING {
#if TR_VERSION == 1
    ROOM_MESH mesh;
#elif TR_VERSION == 2
    int16_t *data;
#endif
    PORTALS *portals;
    SECTOR *sectors;
    LIGHT *lights;
    STATIC_MESH *static_meshes;
    XYZ_32 pos;
    int32_t min_floor;
    int32_t max_ceiling;
    struct __PACKING {
        int16_t z;
        int16_t x;
    } size;
#if TR_VERSION == 1
    int16_t ambient;
#else
    int16_t ambient_1;
    int16_t ambient_2;
    int16_t light_mode;
#endif
    int16_t num_lights;
    int16_t num_static_meshes;
    int16_t bound_left;
    int16_t bound_right;
    int16_t bound_top;
    int16_t bound_bottom;
    uint16_t bound_active;
#if TR_VERSION == 2
    int16_t test_left;
    int16_t test_right;
    int16_t test_top;
    int16_t test_bottom;
#endif
    int16_t item_num;
    int16_t fx_num;
    int16_t flipped_room;
    uint16_t flags;
} ROOM;
