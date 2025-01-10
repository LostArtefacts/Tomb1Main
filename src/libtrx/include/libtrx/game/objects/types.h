#pragma once

#include "../anims/types.h"
#include "../collision.h"
#include "../items/types.h"
#include "../types.h"

#include <stdint.h>

typedef struct {
    void *priv;
    XYZ_16 center;
    int32_t radius;
    int16_t num_lights;
    int16_t num_vertices;
    int16_t num_tex_face4s;
    int16_t num_tex_face3s;
    int16_t num_flat_face4s;
    int16_t num_flat_face3s;
    union {
        XYZ_16 *normals;
        int16_t *lights;
    } lighting;
    XYZ_16 *vertices;
    FACE4 *tex_face4s;
    FACE3 *tex_face3s;
    FACE4 *flat_face4s;
    FACE3 *flat_face3s;
    bool enable_reflections;
} OBJECT_MESH;

#if TR_VERSION == 1
typedef struct {
    struct {
        XYZ_16 min;
        XYZ_16 max;
    } shift, rot;
} OBJECT_BOUNDS;

typedef struct {
    int16_t mesh_count;
    int16_t mesh_idx;
    int32_t bone_idx;
    uint32_t frame_ofs;
    ANIM_FRAME *frame_base;
    void (*initialise)(int16_t item_num);
    void (*control)(int16_t item_num);
    int16_t (*floor_height_func)(
        const ITEM *item, int32_t x, int32_t y, int32_t z, int16_t height);
    int16_t (*ceiling_height_func)(
        const ITEM *item, int32_t x, int32_t y, int32_t z, int16_t height);
    void (*draw_routine)(ITEM *item);
    void (*collision)(int16_t item_num, ITEM *lara_item, COLL_INFO *coll);
    const OBJECT_BOUNDS *(*bounds)(void);
    int16_t anim_idx;
    int16_t hit_points;
    int16_t pivot_length;
    int16_t radius;
    int16_t smartness;
    int16_t shadow_size;
    uint16_t loaded : 1;
    uint16_t intelligent : 1;
    uint16_t save_position : 1;
    uint16_t save_hitpoints : 1;
    uint16_t save_flags : 1;
    uint16_t save_anim : 1;
} OBJECT;

#elif TR_VERSION == 2
typedef struct {
    int16_t mesh_count;
    int16_t mesh_idx;
    int32_t bone_idx;
    uint32_t frame_ofs;
    ANIM_FRAME *frame_base;

    void (*initialise)(int16_t item_num);
    void (*control)(int16_t item_num);
    void (*floor)(
        const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_height);
    void (*ceiling)(
        const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_height);
    void (*draw_routine)(const ITEM *item);
    void (*collision)(int16_t item_num, ITEM *lara_item, COLL_INFO *coll);

    int16_t anim_idx;
    int16_t hit_points;
    int16_t pivot_length;
    int16_t radius;
    int16_t shadow_size;

    union {
        uint16_t flags;
        // clang-format off
        struct {
            uint16_t loaded:           1; // 0x01 1
            uint16_t intelligent:      1; // 0x02 2
            uint16_t save_position:    1; // 0x04 4
            uint16_t save_hitpoints:   1; // 0x08 8
            uint16_t save_flags:       1; // 0x10 16
            uint16_t save_anim:        1; // 0x20 32
            uint16_t semi_transparent: 1; // 0x40 64
            uint16_t water_creature:   1; // 0x80 128
            uint16_t pad:              8;
        };
        // clang-format on
    };
} OBJECT;
#endif
