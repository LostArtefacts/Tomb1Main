#pragma once

#include <stdint.h>

typedef struct STATS_COMMON {
    uint32_t timer;
    uint32_t kill_count;
    uint16_t secret_count;
    uint16_t pickup_count;
    uint32_t max_kill_count;
    uint16_t max_secret_count;
    uint16_t max_pickup_count;
} STATS_COMMON;

typedef struct {
    struct STATS_COMMON;
    uint16_t secret_flags;
} LEVEL_STATS;

typedef struct {
    struct STATS_COMMON;
} FINAL_STATS;
