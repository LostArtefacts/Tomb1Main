#pragma once

#include <stdint.h>

typedef enum {
    GONDOLA_STATE_EMPTY = 0,
    GONDOLA_STATE_FLOATING = 1,
    GONDOLA_STATE_CRASH = 2,
    GONDOLA_STATE_SINK = 3,
    GONDOLA_STATE_LAND = 4,
} GONDOLA_STATE;

void __cdecl Gondola_Control(int16_t item_num);
void Gondola_Setup(void);
