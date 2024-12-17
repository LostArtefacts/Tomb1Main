#pragma once

#include "global/types.h"

#include <libtrx/config.h>
#include <libtrx/gfx/common.h>
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
        bool enable_fade_effects;
        bool fix_item_rots;
    } visuals;

    struct {
        bool fix_m16_accuracy;
        bool fix_item_duplication_glitch;
        bool fix_floor_data_issues;
        bool fix_flare_throw_priority;
        bool enable_cheats;
        bool enable_console;
        bool enable_auto_item_selection;
        int32_t turbo_speed;
    } gameplay;

    struct {
        int32_t sound_volume;
        int32_t music_volume;
        bool enable_lara_mic;
    } audio;

    struct {
        bool is_fullscreen;
        bool is_maximized;
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
    } window;

    struct {
        RENDER_MODE render_mode;
        ASPECT_MODE aspect_mode;
        bool enable_zbuffer;
        bool enable_perspective_filter;
        bool enable_wireframe;
        float wireframe_width;
        GFX_TEXTURE_FILTER texture_filter;
        SCREENSHOT_FORMAT screenshot_format;
        LIGHTING_CONTRAST lighting_contrast;
        TEXEL_ADJUST_MODE texel_adjust_mode;
        int32_t nearest_adjustment;
        int32_t linear_adjustment;
        int32_t scaler;
        float sizer;
    } rendering;
} CONFIG;

extern CONFIG g_Config;
extern CONFIG g_SavedConfig;
