#pragma once

#include "./enum.h"

typedef struct {
    const struct CONSOLE_COMMAND *cmd;
    const char *prefix;
    const char *args;
} COMMAND_CONTEXT;

typedef struct CONSOLE_COMMAND {
    const char *prefix;
    COMMAND_RESULT (*proc)(const COMMAND_CONTEXT *ctx);
} CONSOLE_COMMAND;
