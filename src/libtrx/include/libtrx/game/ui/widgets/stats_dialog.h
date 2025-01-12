#pragma once

#include "./base.h"

typedef enum {
    UI_STATS_DIALOG_MODE_LEVEL,
    UI_STATS_DIALOG_MODE_FINAL,
#if TR_VERSION == 2
    UI_STATS_DIALOG_MODE_ASSAULT_COURSE,
#endif
} UI_STATS_DIALOG_MODE;

typedef enum {
    UI_STATS_DIALOG_STYLE_BARE,
    UI_STATS_DIALOG_STYLE_BORDERED,
} UI_STATS_DIALOG_STYLE;

typedef struct {
    UI_STATS_DIALOG_MODE mode;
    UI_STATS_DIALOG_STYLE style;
    int32_t level_num;
} UI_STATS_DIALOG_ARGS;

UI_WIDGET *UI_StatsDialog_Create(UI_STATS_DIALOG_ARGS args);
