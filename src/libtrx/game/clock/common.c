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
    const Uint64 counter = SDL_GetPerformanceCounter();

    if (m_LastCounter == 0) {
        m_LastCounter = counter;
        return 1;
    }

    const int32_t fps = Clock_GetCurrentFPS();
    const double speed_multiplier = Clock_GetSpeedMultiplier();
    const double frame_duration =
        (1.0 / (fps * speed_multiplier)) * SDL_GetPerformanceFrequency();
    const double elapsed = (double)(counter - m_LastCounter);

    int32_t elapsed_frames = (int32_t)(elapsed / frame_duration);
    if (elapsed_frames < 1) {
        const Uint32 delay_time =
            (frame_duration - elapsed) * 1000 / SDL_GetPerformanceFrequency();
        SDL_Delay(delay_time);
        elapsed_frames = 1;
    }

    m_LastCounter = SDL_GetPerformanceCounter();

    return elapsed_frames;
}
