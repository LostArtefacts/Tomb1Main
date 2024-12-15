#include "game/clock.h"

#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

static Uint64 m_LastCounter = 0;
static Uint64 m_InitCounter = 0;
static Uint64 m_Frequency = 0;
static double m_Accumulator = 0.0;

void Clock_Init(void)
{
    m_Frequency = SDL_GetPerformanceFrequency();
    m_InitCounter = SDL_GetPerformanceCounter();
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

void Clock_SyncTick(void)
{
    m_LastCounter = SDL_GetPerformanceCounter();
    m_Accumulator = 0.0;
}

int32_t Clock_WaitTick(void)
{
    const Uint64 current_counter = SDL_GetPerformanceCounter();

    // If this is the first call, just initialize and return a frame.
    if (m_LastCounter == 0) {
        m_LastCounter = current_counter;
        return 1;
    }

    const int32_t fps = Clock_GetCurrentFPS();
    const double speed_multiplier = Clock_GetSpeedMultiplier();

    // The duration of one frame in performance counter units
    const double frame_ticks = m_Frequency / (fps * speed_multiplier);

    // Calculate elapsed ticks since last call
    const double elapsed_ticks = (double)(current_counter - m_LastCounter);

    // Add the elapsed ticks to the accumulator
    m_Accumulator += elapsed_ticks;

    // Determine how many frames we can "release" from the accumulator
    int32_t frames = (int32_t)(m_Accumulator / frame_ticks);

    if (frames < 1) {
        // Not enough accumulated time for even one frame

        // Calculate how long we should wait (in ms) to hit the frame boundary
        double needed = frame_ticks - m_Accumulator;
        double delay_ms = (needed / m_Frequency) * 1000.0;

        if (delay_ms > 0) {
            SDL_Delay((Uint32)delay_ms);
        }

        // After waiting, measure again to be accurate
        const Uint64 after_delay_counter = SDL_GetPerformanceCounter();
        const double after_delay_elapsed =
            (double)(after_delay_counter - current_counter);
        m_Accumulator += after_delay_elapsed;

        // Now, we should have at least one frame available
        frames = (int32_t)(m_Accumulator / frame_ticks);
        if (frames < 1) {
            // To avoid a possible floating-point corner case, ensure at least
            // one frame
            frames = 1;
        }
    }

    // Consume the frames from the m_Accumulator
    m_Accumulator -= frames * frame_ticks;

    // Update the last counter to the current performance counter
    m_LastCounter = SDL_GetPerformanceCounter();

    return frames;
}
