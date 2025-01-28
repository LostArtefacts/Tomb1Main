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

    struct {
        int32_t object_count;
        int32_t sprite_count;
        int32_t page_count;
        uint8_t *pages_24;
        RGBA_8888 *pages_32;
    } textures;

    struct {
        int32_t size;
        RGB_888 *data_24;
    } palette;
    int32_t mesh_ptr_count;
    int32_t item_count;
    int32_t sample_info_count;
    int32_t sample_count;
    int32_t *sample_offsets;
    int32_t sample_data_size;
    char *sample_data;
} LEVEL_INFO;
