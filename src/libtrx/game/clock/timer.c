#include "debug.h"
#include "game/clock.h"

#include <stdio.h>

static double M_GetElapsedUnit(CLOCK_TIMER *const timer, const double unit);
static bool M_CheckElapsedUnit(
    CLOCK_TIMER *const timer, const double how_often, const double unit,
    bool bypass_turbo_cheat);

static double M_GetElapsedUnit(CLOCK_TIMER *const timer, const double unit)
{
    ASSERT(timer != NULL);
    const double delta = Clock_GetHighPrecisionCounter() - timer->prev_counter;
    const double multiplier = Clock_GetSpeedMultiplier() / 1000.0;
    const double frames = delta * multiplier * unit;
    Clock_ResetTimer(timer);
    return frames;
}

static bool M_CheckElapsedUnit(
    CLOCK_TIMER *const timer, const double how_often, const double unit,
    bool bypass_turbo_cheat)
{
    ASSERT(timer != NULL);
    const double delta = Clock_GetHighPrecisionCounter() - timer->prev_counter;
    const double multiplier =
        (bypass_turbo_cheat ? 1.0 : Clock_GetSpeedMultiplier()) / 1000.0;
    const double frames = delta * multiplier * unit;
    if (Clock_GetCurrentFPS() != timer->prev_fps) {
        Clock_ResetTimer(timer);
        return false;
    }
    if (frames >= how_often) {
        Clock_ResetTimer(timer);
        return true;
    }
    return false;
}

void Clock_ResetTimer(CLOCK_TIMER *const timer)
{
    ASSERT(timer != NULL);
    timer->prev_counter = Clock_GetHighPrecisionCounter();
    timer->prev_fps = Clock_GetCurrentFPS();
}

double Clock_GetElapsedLogicalFrames(CLOCK_TIMER *const timer)
{
    return M_GetElapsedUnit(timer, LOGIC_FPS);
}

double Clock_GetElapsedDrawFrames(CLOCK_TIMER *const timer)
{
    return M_GetElapsedUnit(timer, Clock_GetCurrentFPS());
}

double Clock_GetElapsedMilliseconds(CLOCK_TIMER *const timer)
{
    return M_GetElapsedUnit(timer, 1000.0);
}

bool Clock_CheckElapsedLogicalFrames(
    CLOCK_TIMER *const timer, const int32_t how_often)
{
    return M_CheckElapsedUnit(timer, how_often, LOGIC_FPS, false);
}

bool Clock_CheckElapsedDrawFrames(
    CLOCK_TIMER *const timer, const int32_t how_often)
{
    return M_CheckElapsedUnit(timer, how_often, Clock_GetCurrentFPS(), false);
}

bool Clock_CheckElapsedMilliseconds(
    CLOCK_TIMER *const timer, const int32_t how_often)
{
    return M_CheckElapsedUnit(timer, how_often, 1000, false);
}

bool Clock_CheckElapsedRawMilliseconds(
    CLOCK_TIMER *const timer, const int32_t how_often)
{
    return M_CheckElapsedUnit(timer, how_often, 1000, true);
}
