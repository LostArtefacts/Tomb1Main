#pragma once

#include <libtrx/game/clock.h>

#include <stdbool.h>
#include <stdint.h>

#define CLOCK_TURBO_SPEED_MIN -2
#define CLOCK_TURBO_SPEED_MAX 2

int32_t Clock_GetTurboSpeed(void);
void Clock_CycleTurboSpeed(bool forward);
void Clock_SetTurboSpeed(int32_t value);
