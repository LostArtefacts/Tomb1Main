#pragma once

#include "./enum.h"

#include <stdint.h>

typedef struct GAME_FLOW_COMMAND {
    GAME_FLOW_ACTION action;
    int32_t param;
} GAME_FLOW_COMMAND;
