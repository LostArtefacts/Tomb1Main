#pragma once

#include <stdint.h>

#define CLOCK_TURBO_SPEED_MIN -2
#define CLOCK_TURBO_SPEED_MAX 2

void Clock_CycleTurboSpeed(bool forward);

extern int32_t Clock_GetTurboSpeed(void);
extern void Clock_SetTurboSpeed(int32_t value);

double Clock_GetSpeedMultiplier(void);
