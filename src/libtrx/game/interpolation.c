#include "game/interpolation.h"

#include "config.h"

#include <stdint.h>

static bool m_IsEnabled = true;
static double m_Rate = 0.0;

static int32_t M_GetFPS(void)
{
#if TR_VERSION == 1
    return g_Config.rendering.fps;
#elif TR_VERSION == 2
    return 30;
#endif
}

bool Interpolation_IsEnabled(void)
{
    return m_IsEnabled && M_GetFPS() == 60;
}

void Interpolation_Disable(void)
{
    m_IsEnabled = false;
}

void Interpolation_Enable(void)
{
    m_IsEnabled = true;
}

double Interpolation_GetRate(void)
{
    if (!Interpolation_IsEnabled()) {
        return 1.0;
    }
    return m_Rate;
}

void Interpolation_SetRate(double rate)
{
    m_Rate = rate;
}
