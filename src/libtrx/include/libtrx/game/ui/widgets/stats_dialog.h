#pragma once

#include "./base.h"

typedef enum {
    UI_STATS_DIALOG_MODE_LEVEL,
    UI_STATS_DIALOG_MODE_FINAL,
#if TR_VERSION == 2
    UI_STATS_DIALOG_MODE_ASSAULT_COURSE,
#endif
} UI_STATS_DIALOG_MODE;

UI_WIDGET *UI_StatsDialog_Create(UI_STATS_DIALOG_MODE mode, int32_t level_num);
