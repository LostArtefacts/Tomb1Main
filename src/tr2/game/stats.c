#include "game/stats.h"

#include "game/clock.h"
#include "global/vars.h"

static double m_StartCounter = 0.0;
static int32_t m_StartTimer = 0;

void Stats_StartTimer(void)
{
    m_StartCounter = Clock_GetHighPrecisionCounter();
    m_StartTimer = g_SaveGame.statistics.timer;
}

void Stats_UpdateTimer(void)
{
    const double elapsed =
        (Clock_GetHighPrecisionCounter() - m_StartCounter) * LOGIC_FPS / 1000.0;
    g_SaveGame.statistics.timer = m_StartTimer + elapsed;
}
