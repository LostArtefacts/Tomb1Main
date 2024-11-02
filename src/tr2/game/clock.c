#include "game/clock.h"

#include <libtrx/game/const.h>

#include <windows.h>

double Clock_GetHighPrecisionCounter(void)
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * 1000.0 / frequency.QuadPart;
}

int32_t Clock_GetLogicalFrame(void)
{
    return Clock_GetHighPrecisionCounter() * LOGIC_FPS / 1000.0;
}
