#include "game/clock.h"

#include "game/console/common.h"
#include "game/game_string.h"

#include <libtrx/config.h>
#include <libtrx/utils.h>

int32_t Clock_GetTurboSpeed(void)
{
    return g_Config.gameplay.turbo_speed;
}

void Clock_SetTurboSpeed(int32_t value)
{
    CLAMP(value, CLOCK_TURBO_SPEED_MIN, CLOCK_TURBO_SPEED_MAX);
    if (value == g_Config.gameplay.turbo_speed) {
        return;
    }
    g_Config.gameplay.turbo_speed = value;
    Config_Write();
    Console_Log(GS(OSD_SPEED_SET), value);
    Clock_SetSimSpeed(Clock_GetSpeedMultiplier());
}

int32_t Clock_GetCurrentFPS(void)
{
    return g_Config.rendering.fps;
}
