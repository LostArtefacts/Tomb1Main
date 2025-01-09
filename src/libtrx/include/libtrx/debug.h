#pragma once

#include "log.h"

#define ASSERT(x)                                                              \
    do {                                                                       \
        if (!(x)) {                                                            \
            LOG_DEBUG("Assertion failed: %s", #x);                             \
            __builtin_trap();                                                  \
        }                                                                      \
    } while (0)

#define ASSERT_FAIL(x)                                                         \
    do {                                                                       \
        LOG_DEBUG("Assertion failed");                                         \
        __builtin_trap();                                                      \
    } while (0)
