#include "game/scaler.h"

#include "config.h"
#include "global/vars.h"

#include <libtrx/log.h>
#include <libtrx/utils.h>

static int32_t M_DoCalc(
    int32_t unit, int32_t base_width, int32_t base_height, double factor)
{
    int32_t scale_x = g_PhdWinWidth > base_width
        ? ((double)g_PhdWinWidth * unit * factor) / base_width
        : unit * factor;
    int32_t scale_y = g_PhdWinHeight > base_height
        ? ((double)g_PhdWinHeight * unit * factor) / base_height
        : unit * factor;
    return MIN(scale_x, scale_y);
}

int32_t Scaler_Calc(const int32_t unit, const SCALER_TARGET target)
{
    double scale = 1.0;
    switch (target) {
    case SCALER_TARGET_BAR:
        scale = g_Config.ui.bar_scale;
        break;
    }

    return M_DoCalc(unit, 640, 480, scale);
}

int32_t Scaler_CalcInverse(const int32_t unit, const SCALER_TARGET target)
{
    return unit * 0x10000 / MAX(1, Scaler_Calc(0x10000, target));
}
