#pragma once

#include "./types.h"

#include <stdint.h>

typedef struct {
    const char *file_name;
    int32_t display_time;
    bool display_time_includes_fades;
    int32_t fade_in_time;
    int32_t fade_out_time;
} PHASE_PICTURE_ARGS;

PHASE *Phase_Picture_Create(PHASE_PICTURE_ARGS args);
void Phase_Picture_Destroy(PHASE *phase);
