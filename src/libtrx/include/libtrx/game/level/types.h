#pragma once

#include "../output/types.h"

typedef struct {
    int32_t mesh_count;
    int32_t mesh_ptr_count;
    int32_t anim_count;
    int32_t anim_change_count;
    int32_t anim_range_count;
    int32_t anim_command_count;
    int32_t anim_bone_count;
    int32_t anim_frame_data_count;
    int16_t *anim_frame_data;
    int32_t texture_count;
    int32_t texture_page_count;
    uint8_t *texture_palette_page_ptrs;
    RGBA_8888 *texture_rgb_page_ptrs;
    int32_t item_count;
    int32_t sprite_info_count;
    int32_t overlap_count;
    int32_t sample_info_count;
    int32_t sample_count;
    int32_t *sample_offsets;
    int32_t sample_data_size;
    char *sample_data;
    RGB_888 *palette;
    int32_t palette_size;
} LEVEL_INFO;
