#pragma once

#include "global/types.h"

typedef struct {
    int32_t count;
    char **data_paths;
} INJECTION_DATA;

typedef struct {
    struct {
        GAME_FLOW_LEVEL_TYPE type;
        int32_t num;
    } current_level;
} GAME_INFO;

typedef struct {
    const char *key;
    const char *value;
} GAME_FLOW_NEW_STRING_ENTRY;

typedef struct {
    GAME_FLOW_NEW_STRING_ENTRY *object_strings;
    GAME_FLOW_NEW_STRING_ENTRY *game_strings;
    INJECTION_DATA injections;
} GAME_FLOW_NEW_LEVEL;

typedef struct {
    int32_t level_count;
    GAME_FLOW_NEW_LEVEL *levels;
    GAME_FLOW_NEW_STRING_ENTRY *object_strings;
    GAME_FLOW_NEW_STRING_ENTRY *game_strings;
    INJECTION_DATA injections;
} GAME_FLOW_NEW;

extern GAME_FLOW_NEW g_GameFlowNew;
extern GAME_INFO g_GameInfo;

void GF_N_LoadStrings(int32_t level_num);
