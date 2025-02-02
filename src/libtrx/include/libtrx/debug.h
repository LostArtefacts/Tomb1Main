#pragma once

#include "log.h"

#define ASSERT(x)                                                              \
    do {                                                                       \
        if (!(x)) {                                                            \
            LOG_DEBUG("Assertion failed: %s", #x);                             \
            __builtin_trap();                                                  \
        }                                                                      \
    } while (0)

#define ASSERT_FAIL()                                                          \
    do {                                                                       \
        LOG_DEBUG("Assertion failed");                                         \
        __builtin_trap();                                                      \
    } while (0)

#define ASSERT_FAIL_FMT(fmt, ...)                                              \
    do {                                                                       \
        LOG_DEBUG("Assertion failed: " fmt __VA_OPT__(, ) __VA_ARGS__);        \
        __builtin_trap();                                                      \
    } while (0)
