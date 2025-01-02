#pragma once

#include "../math.h"

typedef struct __PACKING {
    int16_t goal_anim_state;
    int16_t num_ranges;
    int16_t range_idx;
} ANIM_CHANGE;

typedef struct __PACKING {
    int16_t start_frame;
    int16_t end_frame;
    int16_t link_anim_num;
    int16_t link_frame_num;
} ANIM_RANGE;

#if TR_VERSION > 1
typedef struct {
    union {
        int32_t flags;
        // clang-format off
        struct {
            uint32_t matrix_pop:  1;
            uint32_t matrix_push: 1;
            uint32_t rot_x:       1;
            uint32_t rot_y:       1;
            uint32_t rot_z:       1;
            uint32_t pad:         11;
        };
        // clang-format on
    };
    XYZ_32 pos;
} BONE;
#endif

typedef struct __PACKING {
    BOUNDS_16 bounds;
    XYZ_16 offset;
#if TR_VERSION == 1
    int16_t nmeshes;
    int32_t *mesh_rots;
#else
    int16_t mesh_rots[];
#endif
} FRAME_INFO;

typedef struct __PACKING {
#if TR_VERSION == 1
    FRAME_INFO *frame_ptr;
    uint32_t frame_ofs;
#elif TR_VERSION == 2
    int16_t *frame_ptr;
#endif
    int16_t interpolation;
    int16_t current_anim_state;
    int32_t velocity;
    int32_t acceleration;
    int16_t frame_base;
    int16_t frame_end;
    int16_t jump_anim_num;
    int16_t jump_frame_num;
    int16_t num_changes;
    int16_t change_idx;
    int16_t num_commands;
    int16_t command_idx;
} ANIM;
