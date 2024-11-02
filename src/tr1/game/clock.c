#include "game/clock.h"

#include "config.h"
#include "game/console/common.h"
#include "game/game_string.h"
#include "global/const.h"

#include <libtrx/utils.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>

int32_t Clock_GetTurboSpeed(void)
{
    return g_Config.rendering.turbo_speed;
}

void Clock_CycleTurboSpeed(const bool forward)
{
    Clock_SetTurboSpeed(g_Config.rendering.turbo_speed + (forward ? 1 : -1));
}

void Clock_SetTurboSpeed(int32_t value)
{
    CLAMP(value, CLOCK_TURBO_SPEED_MIN, CLOCK_TURBO_SPEED_MAX);
    g_Config.rendering.turbo_speed = value;
    Config_Write();
    Console_Log(GS(OSD_SPEED_SET), value);
}

double Clock_GetSpeedMultiplier(void)
{
    if (g_Config.rendering.turbo_speed > 0) {
        return 1.0 + g_Config.rendering.turbo_speed;
    } else if (g_Config.rendering.turbo_speed < 0) {
        return pow(2.0, g_Config.rendering.turbo_speed);
    } else {
        return 1.0;
    }
}

int32_t Clock_GetCurrentFPS(void)
{
    return g_Config.rendering.fps;
}
