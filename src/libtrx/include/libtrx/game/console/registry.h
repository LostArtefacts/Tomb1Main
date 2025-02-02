#pragma once

#include "./types.h"

void Console_Registry_Add(CONSOLE_COMMAND cmd);
const CONSOLE_COMMAND *Console_Registry_Get(const char *cmdline);

#define REGISTER_CONSOLE_COMMAND(prefix_, proc_)                               \
    __attribute__((__constructor__)) static void M_Register(void)              \
    {                                                                          \
        Console_Registry_Add(                                                  \
            (CONSOLE_COMMAND) { .prefix = prefix_, .proc = proc_ });           \
    }
