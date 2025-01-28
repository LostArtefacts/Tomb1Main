#pragma once

#include "../output/types.h"

typedef struct {
    struct {
        int32_t anim_count;
        int32_t change_count;
        int32_t range_count;
        int32_t command_count;
        int32_t bone_count;
        int32_t frame_count;
        int16_t *frames;
    } anims;
    int32_t mesh_ptr_count;
    int32_t texture_count;
    int32_t texture_page_count;
    uint8_t *texture_palette_page_ptrs;
    RGBA_8888 *texture_rgb_page_ptrs;
    int32_t item_count;
    int32_t sprite_info_count;
    int32_t sample_info_count;
    int32_t sample_count;
    int32_t *sample_offsets;
    int32_t sample_data_size;
    char *sample_data;
    RGB_888 *palette;
    int32_t palette_size;
} LEVEL_INFO;
