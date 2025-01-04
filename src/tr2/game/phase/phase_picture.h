#pragma once

#include <libtrx/game/phase.h>

#include <stdint.h>

typedef struct {
    const char *file_name;
    int32_t display_time;
    int32_t fade_in_time;
    int32_t fade_out_time;
} PHASE_PICTURE_ARGS;

PHASE *Phase_Picture_Create(PHASE_PICTURE_ARGS args);
void Phase_Picture_Destroy(PHASE *phase);
