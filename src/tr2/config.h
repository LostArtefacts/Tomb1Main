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
        bool enable_cheats;
        bool fix_item_duplication_glitch;
        bool enable_auto_item_selection;
        bool fix_floor_data_issues;
    } gameplay;

    struct {
        int32_t turbo_speed;
        SCREENSHOT_FORMAT screenshot_format;
    } rendering;
} CONFIG;

extern CONFIG g_Config;
