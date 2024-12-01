#pragma once

#include "log.h"

#define ASSERT(x)                                                              \
    do {                                                                       \
        if (!(x)) {                                                            \
            LOG_DEBUG("Assertion failed: %s", #x);                             \
            *(int *)0 = 0;                                                     \
        }                                                                      \
    } while (0)

#define ASSERT_FAIL(x)                                                         \
    do {                                                                       \
        LOG_DEBUG("Assertion failed");                                         \
        *(int *)0 = 0;                                                         \
    } while (0)
