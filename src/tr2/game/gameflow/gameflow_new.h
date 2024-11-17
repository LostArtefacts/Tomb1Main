#pragma once

#include "global/types.h"

typedef struct {
    int32_t count;
    char **data_paths;
} INJECTION_DATA;

typedef struct {
    struct {
        GAMEFLOW_LEVEL_TYPE type;
        int32_t num;
    } current_level;
} GAME_INFO;

typedef struct {
    const char *key;
    const char *value;
} GAMEFLOW_NEW_STRING_ENTRY;

typedef struct {
    GAMEFLOW_NEW_STRING_ENTRY *object_strings;
    GAMEFLOW_NEW_STRING_ENTRY *game_strings;
    INJECTION_DATA injections;
} GAMEFLOW_NEW_LEVEL;

typedef struct {
    int32_t level_count;
    GAMEFLOW_NEW_LEVEL *levels;
    GAMEFLOW_NEW_STRING_ENTRY *object_strings;
    GAMEFLOW_NEW_STRING_ENTRY *game_strings;
    INJECTION_DATA injections;
} GAMEFLOW_NEW;

extern GAMEFLOW_NEW g_GameflowNew;
extern GAME_INFO g_GameInfo;

void GF_N_LoadStrings(int32_t level_num);
