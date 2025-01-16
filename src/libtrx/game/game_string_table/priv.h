#pragma once

#include <stdint.h>

typedef struct {
    const char *key;
    const char *value;
} GS_TABLE_ENTRY;

typedef struct {
    GS_TABLE_ENTRY *object_names;
    GS_TABLE_ENTRY *object_descriptions;
    GS_TABLE_ENTRY *game_strings;
} GS_TABLE;

typedef struct {
    int32_t level_count;
    GS_TABLE global;
    GS_TABLE *levels;
} GS_FILE;

extern GS_FILE g_GST_File;
