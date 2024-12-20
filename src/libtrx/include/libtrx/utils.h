#pragma once

#define Q(x) #x
#define QUOTE(x) Q(x)

#define SQUARE(A) ((A) * (A))
#ifndef ABS
    #define ABS(x) (((x) < 0) ? (-(x)) : (x))
    #define MIN(x, y) ((x) <= (y) ? (x) : (y))
    #define MAX(x, y) ((x) >= (y) ? (x) : (y))
#endif

#define CLAMPL(a, b)                                                           \
    do {                                                                       \
        if ((a) < (b))                                                         \
            (a) = (b);                                                         \
    } while (0)
#define CLAMPG(a, b)                                                           \
    do {                                                                       \
        if ((a) > (b))                                                         \
            (a) = (b);                                                         \
    } while (0)
#define CLAMP(a, b, c)                                                         \
    do {                                                                       \
        if ((a) < (b))                                                         \
            (a) = (b);                                                         \
        else if ((a) > (c))                                                    \
            (a) = (c);                                                         \
    } while (0)
#define SWAP(a, b, c)                                                          \
    do {                                                                       \
        (c) = (a);                                                             \
        (a) = (b);                                                             \
        (b) = (c);                                                             \
    } while (0)

#define ALIGN(a, bytes) ((a + (bytes) - 1) & (~(bytes - 1)))
#define TOGGLE_BIT(target_var, target_bit, condition)                          \
    do {                                                                       \
        if (condition) {                                                       \
            (target_var) |= (target_bit);                                      \
        } else {                                                               \
            (target_var) &= ~(target_bit);                                     \
        }                                                                      \
    } while (0)

#define MKTAG(a, b, c, d)                                                      \
    ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

#if TR_VERSION == 1
    #define PROJECT_NAME "TR1X"
#elif TR_VERSION == 2
    #define PROJECT_NAME "TR2X"
#endif
