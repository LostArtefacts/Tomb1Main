#pragma once

#include <libtrx/game/ui/widgets/base.h>

typedef enum {
    UI_STATS_DIALOG_MODE_LEVEL,
    UI_STATS_DIALOG_MODE_FINAL,
    UI_STATS_DIALOG_MODE_ASSAULT_COURSE,
} UI_STATS_DIALOG_MODE;

UI_WIDGET *UI_StatsDialog_Create(UI_STATS_DIALOG_MODE mode);
