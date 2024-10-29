#pragma once

#include <libtrx/config.h>
#include <libtrx/screenshot.h>

#include <stdbool.h>

typedef struct {
    bool loaded;

    struct {
        bool fix_m16_accuracy;
    } gameplay;

    struct {
        SCREENSHOT_FORMAT screenshot_format;
    } rendering;
} CONFIG;

extern CONFIG g_Config;
