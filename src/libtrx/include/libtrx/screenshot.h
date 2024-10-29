#pragma once

#include <stdbool.h>

typedef enum {
    SCREENSHOT_FORMAT_JPEG,
    SCREENSHOT_FORMAT_PNG,
} SCREENSHOT_FORMAT;

bool Screenshot_Make(SCREENSHOT_FORMAT format);
