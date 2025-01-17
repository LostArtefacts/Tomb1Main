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
    INJECTION_DATA injections;
} GAME_FLOW_LEVEL;

typedef struct {
    int32_t level_count;
    GAME_FLOW_LEVEL *levels;
    INJECTION_DATA injections;
} GAME_FLOW;

extern GAME_FLOW g_GameFlow;
extern GAME_INFO g_GameInfo;
