#include "game/clock.h"

#include <libtrx/game/const.h>

double Clock_GetSpeedMultiplier(void)
{
    return 1.0;
}

int32_t Clock_GetCurrentFPS(void)
{
    return LOGIC_FPS;
}
