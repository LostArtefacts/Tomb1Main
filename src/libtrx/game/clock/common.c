#include "game/clock.h"

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

static Uint64 m_LastCounter = 0;
static Uint64 m_InitCounter = 0;
static Uint64 m_Counter = 0;
static Uint64 m_Frequency = 0;

void Clock_Init(void)
{
    m_Frequency = SDL_GetPerformanceFrequency();
    m_InitCounter = SDL_GetPerformanceCounter();
    m_Counter = SDL_GetPerformanceCounter();
}

size_t Clock_GetDateTime(char *const buffer, const size_t size)
{
    time_t lt = time(0);
    struct tm *tptr = localtime(&lt);

    return snprintf(
        buffer, size, "%04d%02d%02d_%02d%02d%02d", tptr->tm_year + 1900,
        tptr->tm_mon + 1, tptr->tm_mday, tptr->tm_hour, tptr->tm_min,
        tptr->tm_sec);
}

int32_t Clock_GetFrameAdvance(void)
{
    return Clock_GetCurrentFPS() == 30 ? 2 : 1;
}

int32_t Clock_GetLogicalFrame(void)
{
    return Clock_GetHighPrecisionCounter() * LOGIC_FPS / 1000.0;
}

int32_t Clock_GetDrawFrame(void)
{
    return Clock_GetHighPrecisionCounter() * Clock_GetCurrentFPS() / 1000.0;
}

double Clock_GetHighPrecisionCounter(void)
{
    return (SDL_GetPerformanceCounter() - m_InitCounter) * 1000.0
        / (double)m_Frequency;
}

int32_t Clock_SyncTicks(void)
{
    m_LastCounter = m_Counter;
    const double fps = Clock_GetCurrentFPS();

    const double frequency = (double)m_Frequency / Clock_GetSpeedMultiplier();
    const Uint64 target_counter = m_LastCounter + (frequency / fps);

    while (true) {
        m_Counter = SDL_GetPerformanceCounter();

        const double elapsed_sec =
            (double)(m_Counter - m_LastCounter) / frequency;
        const double delay_sec = m_Counter <= target_counter
            ? (double)(target_counter - m_Counter) / frequency
            : 0.0;
        int32_t delay_ms = delay_sec * 1000;
        if (delay_ms > 0) {
            SDL_Delay(delay_ms);
        }

        const double elapsed_ticks = elapsed_sec * fps;
        if (elapsed_ticks >= 1) {
            return elapsed_ticks;
        }
    }
}
