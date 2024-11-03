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
        bool fix_m16_accuracy;
        bool enable_cheats;
    } gameplay;

    struct {
        int32_t turbo_speed;
        SCREENSHOT_FORMAT screenshot_format;
    } rendering;
} CONFIG;

extern CONFIG g_Config;
