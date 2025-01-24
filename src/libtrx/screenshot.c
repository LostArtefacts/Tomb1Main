#include "screenshot.h"

#include "filesystem.h"
#include "game/clock.h"
#include "game/game.h"
#include "game/game_flow/common.h"
#include "game/output.h"
#include "memory.h"

#include <stdio.h>
#include <string.h>

#define SCREENSHOTS_DIR "screenshots"

static char *M_GetScreenshotTitle(void);
static char *M_CleanScreenshotTitle(const char *source);
static char *M_GetScreenshotBaseName(void);
static const char *M_GetScreenshotFileExt(SCREENSHOT_FORMAT format);
static char *M_GetScreenshotPath(SCREENSHOT_FORMAT format);

static char *M_CleanScreenshotTitle(const char *const source)
{
    // Sanitize screenshot title.
    // - Replace spaces with underscores
    // - Remove all non-alphanumeric characters
    // - Merge consecutive underscores together
    // - Remove leading underscores
    // - Remove trailing underscores
    char *result = Memory_Alloc(strlen(source) + 1);

    bool last_was_underscore = false;
    char *out = result;
    for (size_t i = 0; i < strlen(source); i++) {
        if (source[i] == ' ' || source[i] == '_') {
            if (!last_was_underscore && out > result) {
                *out++ = '_';
                last_was_underscore = true;
            }
        } else if (((source[i] >= 'A' && source[i] <= 'Z')
                    || (source[i] >= 'a' && source[i] <= 'z')
                    || (source[i] >= '0' && source[i] <= '9'))) {
            *out++ = source[i];
            last_was_underscore = false;
        }
    }
    *out++ = '\0';

    // Strip trailing underscores
    while (out[-1] == '_' && out >= result) {
        out--;
    }
    *out = '\0';

    return result;
}

static char *M_GetScreenshotTitle(void)
{
    const GAME_FLOW_LEVEL *const level = GF_GetCurrentLevel();
    if (level == NULL) {
        return Memory_DupStr("Intro");
    }

    if (level->title != NULL && strlen(level->title) > 0) {
        char *clean_level_title = M_CleanScreenshotTitle(level->title);
        if (clean_level_title != NULL && strlen(clean_level_title) > 0) {
            return clean_level_title;
        }
        Memory_FreePointer(clean_level_title);
    }

    // If title totally invalid, name it based on level number
    const char *const fmt = "Level_%d";
    const size_t result_size = snprintf(NULL, 0, fmt, level->num) + 1;
    char *result = Memory_Alloc(result_size);
    snprintf(result, result_size, fmt, level->num);
    return result;
}

static char *M_GetScreenshotBaseName(void)
{
    char *screenshot_title = M_GetScreenshotTitle();

    // Get timestamp
    char date_time[30];
    Clock_GetDateTime(date_time, 30);

    // Full screenshot name
    const char *const fmt = "%s_%s";
    const size_t out_size =
        snprintf(NULL, 0, fmt, date_time, screenshot_title) + 1;
    char *out = Memory_Alloc(out_size);
    snprintf(out, out_size, "%s_%s", date_time, screenshot_title);
    return out;
}

static const char *M_GetScreenshotFileExt(const SCREENSHOT_FORMAT format)
{
    switch (format) {
    case SCREENSHOT_FORMAT_JPEG:
        return "jpg";
    case SCREENSHOT_FORMAT_PNG:
        return "png";
    default:
        return "jpg";
    }
}

static char *M_GetScreenshotPath(const SCREENSHOT_FORMAT format)
{
    char *base_name = M_GetScreenshotBaseName();
    const char *const ext = M_GetScreenshotFileExt(format);

    char *full_path = Memory_Alloc(
        strlen(SCREENSHOTS_DIR) + strlen(base_name) + strlen(ext) + 6);
    sprintf(full_path, "%s/%s.%s", SCREENSHOTS_DIR, base_name, ext);
    if (File_Exists(full_path)) {
        for (int i = 2; i < 100; i++) {
            sprintf(
                full_path, "%s/%s_%d.%s", SCREENSHOTS_DIR, base_name, i, ext);
            if (!File_Exists(full_path)) {
                break;
            }
        }
    }

    Memory_FreePointer(&base_name);
    return full_path;
}

bool Screenshot_Make(const SCREENSHOT_FORMAT format)
{
    File_CreateDirectory(SCREENSHOTS_DIR);

    char *full_path = M_GetScreenshotPath(format);
    const bool result = Output_MakeScreenshot(full_path);
    Memory_FreePointer(&full_path);

    return result;
}
