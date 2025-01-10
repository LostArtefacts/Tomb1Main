#pragma once

#include <libtrx/game/gameflow/types.h>
#include <libtrx/game/ui/widgets/base.h>

typedef enum {
    UI_STATS_DIALOG_MODE_LEVEL,
    UI_STATS_DIALOG_MODE_FINAL,
} UI_STATS_DIALOG_MODE;

UI_WIDGET *UI_StatsDialog_Create(
    UI_STATS_DIALOG_MODE mode, int32_t level_num,
    GAME_FLOW_LEVEL_TYPE level_type);
