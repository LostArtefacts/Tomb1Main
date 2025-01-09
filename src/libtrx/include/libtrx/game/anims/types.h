#pragma once

#include "../math.h"

typedef struct {
    int16_t goal_anim_state;
    int16_t num_ranges;
    int16_t range_idx;
} ANIM_CHANGE;

typedef struct {
    int16_t start_frame;
    int16_t end_frame;
    int16_t link_anim_num;
    int16_t link_frame_num;
} ANIM_RANGE;

typedef struct {
    bool matrix_pop;
    bool matrix_push;
    bool rot_x;
    bool rot_y;
    bool rot_z;
    XYZ_32 pos;
} ANIM_BONE;

typedef struct {
    BOUNDS_16 bounds;
    XYZ_16 offset;
#if TR_VERSION == 1
    XYZ_16 *mesh_rots;
#else
    int16_t mesh_rots[];
#endif
} ANIM_FRAME;

typedef struct {
#if TR_VERSION == 1
    ANIM_FRAME *frame_ptr;
    uint32_t frame_ofs;
#elif TR_VERSION == 2
    int16_t *frame_ptr;
#endif
    uint8_t interpolation;
    uint8_t frame_size;
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
