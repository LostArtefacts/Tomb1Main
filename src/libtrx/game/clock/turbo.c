#include "game/clock/turbo.h"

#include <math.h>

void Clock_CycleTurboSpeed(const bool forward)
{
    Clock_SetTurboSpeed(Clock_GetTurboSpeed() + (forward ? 1 : -1));
}

double Clock_GetSpeedMultiplier(void)
{
    if (Clock_GetTurboSpeed() > 0) {
        return 1.0 + Clock_GetTurboSpeed();
    } else if (Clock_GetTurboSpeed() < 0) {
        return pow(2.0, Clock_GetTurboSpeed());
    } else {
        return 1.0;
    }
}
