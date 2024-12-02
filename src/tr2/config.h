#pragma once

#include <libtrx/config.h>
#include <libtrx/screenshot.h>

#include <stdbool.h>

typedef struct {
    bool loaded;

    struct {
        int32_t keyboard_layout;
        int32_t controller_layout;
    } input;

    struct {
        bool enable_3d_pickups;
        bool fix_item_rots;
    } visuals;

    struct {
        bool fix_m16_accuracy;
        bool fix_item_duplication_glitch;
        bool fix_floor_data_issues;
        bool enable_cheats;
        bool enable_auto_item_selection;
        int32_t turbo_speed;
    } gameplay;

    struct {
        int32_t sound_volume;
        int32_t music_volume;
    } audio;

    struct {
        SCREENSHOT_FORMAT screenshot_format;
        float sizer;
    } rendering;
} CONFIG;

extern CONFIG g_Config;
