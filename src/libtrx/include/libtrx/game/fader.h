#pragma once

#include "clock/timer.h"

#include <stdbool.h>
#include <stdint.h>

#define FADER_ANY (-1)
#define FADER_TRANSPARENT 0
#define FADER_SEMI_BLACK 127
#define FADER_BLACK 255

typedef struct {
    int32_t initial;
    int32_t target;

    // This value controls how much to keep the last frame after the animation
    // is done.
    int32_t debuff;
    int32_t duration;
} FADER_ARGS;

typedef struct {
    FADER_ARGS args;
    CLOCK_TIMER timer;
} FADER;

void Fader_InitEx(FADER *fader, FADER_ARGS args);
void Fader_Init(
    FADER *fader, int32_t initial, int32_t target, int32_t duration);
bool Fader_IsActive(const FADER *fader);
int32_t Fader_GetCurrentValue(const FADER *fader);
