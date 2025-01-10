#include "game/ui/widgets/stats_dialog.h"

#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/ui/common.h>
#include <libtrx/game/ui/widgets/label.h>
#include <libtrx/game/ui/widgets/stack.h>
#include <libtrx/memory.h>

#include <stdio.h>

#define ROW_HEIGHT 30

typedef enum {
    M_ROW_KILLS,
    M_ROW_PICKUPS,
    M_ROW_SECRETS,
    M_ROW_DEATHS,
    M_ROW_TIMER,
} M_ROW_ROLE;

typedef struct {
    M_ROW_ROLE role;
    UI_WIDGET *label;
} M_ROW;

typedef struct {
    UI_WIDGET_VTABLE vtable;
    UI_STATS_DIALOG_MODE mode;
    GAME_FLOW_LEVEL_TYPE level_type;
    int32_t level_num;
    int32_t listener;

    int32_t row_count;
    UI_WIDGET *title;
    UI_WIDGET *stack;
    M_ROW *rows;
} UI_STATS_DIALOG;

static void M_FormatTime(char *out, int32_t total_frames);
static void M_AddRow(UI_STATS_DIALOG *self, M_ROW_ROLE role, const char *text);
static void M_AddRowFromRole(
    UI_STATS_DIALOG *self, M_ROW_ROLE role, const STATS_COMMON *stats);
static void M_AddLevelStatsRows(UI_STATS_DIALOG *self);
static void M_AddFinalStatsRows(
    UI_STATS_DIALOG *self, GAME_FLOW_LEVEL_TYPE level_type);
static void M_UpdateTimerRow(UI_STATS_DIALOG *self);
static void M_DoLayout(UI_STATS_DIALOG *self);
static void M_HandleLayoutUpdate(const EVENT *event, void *data);

static int32_t M_GetWidth(const UI_STATS_DIALOG *self);
static int32_t M_GetHeight(const UI_STATS_DIALOG *self);
static void M_SetPosition(UI_STATS_DIALOG *self, int32_t x, int32_t y);
static void M_Control(UI_STATS_DIALOG *self);
static void M_Draw(UI_STATS_DIALOG *self);
static void M_Free(UI_STATS_DIALOG *self);

static void M_FormatTime(char *const out, const int32_t total_frames)
{
    const int32_t total_seconds = total_frames / LOGIC_FPS;
    const int32_t hours = total_seconds / 3600;
    const int32_t minutes = (total_seconds / 60) % 60;
    const int32_t seconds = total_seconds % 60;
    char time_str[20];
    if (hours != 0) {
        sprintf(time_str, "%d:%02d:%02d", hours, minutes, seconds);
    } else {
        sprintf(time_str, "%d:%02d", minutes, seconds);
    }
    sprintf(out, GS(STATS_TIME_TAKEN_FMT), time_str);
}

static void M_AddRow(
    UI_STATS_DIALOG *const self, const M_ROW_ROLE role, const char *const text)
{
    self->row_count++;
    self->rows = Memory_Realloc(self->rows, sizeof(M_ROW) * self->row_count);
    self->rows[self->row_count - 1].label =
        UI_Label_Create(text, UI_LABEL_AUTO_SIZE, ROW_HEIGHT);
    self->rows[self->row_count - 1].role = role;
    UI_Stack_AddChild(self->stack, self->rows[self->row_count - 1].label);
}

static void M_AddRowFromRole(
    UI_STATS_DIALOG *const self, const M_ROW_ROLE role,
    const STATS_COMMON *const stats)
{
    char buf[50];

    switch (role) {
    case M_ROW_KILLS:
        sprintf(
            buf,
            g_Config.gameplay.enable_detailed_stats ? GS(STATS_KILLS_DETAIL_FMT)
                                                    : GS(STATS_KILLS_BASIC_FMT),
            stats->kill_count, stats->max_kill_count);
        M_AddRow(self, role, buf);
        break;

    case M_ROW_PICKUPS:
        sprintf(
            buf,
            g_Config.gameplay.enable_detailed_stats
                ? GS(STATS_PICKUPS_DETAIL_FMT)
                : GS(STATS_PICKUPS_BASIC_FMT),
            stats->pickup_count, stats->max_pickup_count);
        M_AddRow(self, role, buf);
        break;

    case M_ROW_SECRETS: {
        int secret_count = stats->secret_count;
        sprintf(
            buf, GS(STATS_SECRETS_FMT), secret_count, stats->max_secret_count);
        M_AddRow(self, role, buf);
        break;
    }

    case M_ROW_DEATHS: {
        sprintf(buf, GS(STATS_DEATHS_FMT), stats->death_count);
        M_AddRow(self, role, buf);
        break;
    }

    case M_ROW_TIMER: {
        M_FormatTime(buf, stats->timer);
        M_AddRow(self, role, buf);
        break;
    }

    default:
        break;
    }
}

static void M_AddLevelStatsRows(UI_STATS_DIALOG *const self)
{
    const STATS_COMMON *stats =
        (STATS_COMMON *)&g_GameInfo.current[self->level_num].stats;
    M_AddRowFromRole(self, M_ROW_KILLS, stats);
    M_AddRowFromRole(self, M_ROW_PICKUPS, stats);
    M_AddRowFromRole(self, M_ROW_SECRETS, stats);
    if (g_Config.gameplay.enable_deaths_counter
        && g_GameInfo.death_counter_supported) {
        M_AddRowFromRole(self, M_ROW_DEATHS, stats);
    }
    M_AddRowFromRole(self, M_ROW_TIMER, stats);
}

static void M_AddFinalStatsRows(
    UI_STATS_DIALOG *const self, const GAME_FLOW_LEVEL_TYPE level_type)
{
    FINAL_STATS final_stats;
    Stats_ComputeFinal(self->level_type, &final_stats);
    const STATS_COMMON *stats = (STATS_COMMON *)&final_stats;
    M_AddRowFromRole(self, M_ROW_KILLS, stats);
    M_AddRowFromRole(self, M_ROW_PICKUPS, stats);
    M_AddRowFromRole(self, M_ROW_SECRETS, stats);
    if (g_Config.gameplay.enable_deaths_counter
        && g_GameInfo.death_counter_supported) {
        M_AddRowFromRole(self, M_ROW_DEATHS, stats);
    }
    M_AddRowFromRole(self, M_ROW_TIMER, stats);
}

static void M_UpdateTimerRow(UI_STATS_DIALOG *const self)
{
    if (self->mode != UI_STATS_DIALOG_MODE_LEVEL) {
        return;
    }

    for (int32_t i = 0; i < self->row_count; i++) {
        if (self->rows[i].role != M_ROW_TIMER) {
            continue;
        }
        char buf[50];
        M_FormatTime(buf, g_GameInfo.current[self->level_num].stats.timer);
        UI_Label_ChangeText(self->rows[i].label, buf);
        return;
    }
}

static void M_DoLayout(UI_STATS_DIALOG *const self)
{
    M_SetPosition(
        self, (UI_GetCanvasWidth() - M_GetWidth(self)) / 2,
        (UI_GetCanvasHeight() - M_GetHeight(self)) / 2 + 25);
}

static void M_HandleLayoutUpdate(const EVENT *event, void *data)
{
    UI_STATS_DIALOG *const self = (UI_STATS_DIALOG *)data;
    M_DoLayout(self);
}

static int32_t M_GetWidth(const UI_STATS_DIALOG *const self)
{
    return self->stack->get_width(self->stack);
}

static int32_t M_GetHeight(const UI_STATS_DIALOG *const self)
{
    return self->stack->get_height(self->stack);
}

static void M_SetPosition(
    UI_STATS_DIALOG *const self, const int32_t x, const int32_t y)
{
    self->stack->set_position(self->stack, x, y);
}

static void M_Control(UI_STATS_DIALOG *const self)
{
    if (self->stack->control != NULL) {
        self->stack->control(self->stack);
    }

    M_UpdateTimerRow(self);
}

static void M_Draw(UI_STATS_DIALOG *const self)
{
    if (self->stack->draw != NULL) {
        self->stack->draw(self->stack);
    }
}

static void M_Free(UI_STATS_DIALOG *const self)
{
    self->title->free(self->title);
    for (int32_t i = 0; i < self->row_count; i++) {
        self->rows[i].label->free(self->rows[i].label);
    }
    self->stack->free(self->stack);
    UI_Events_Unsubscribe(self->listener);
    Memory_Free(self);
}

UI_WIDGET *UI_StatsDialog_Create(
    const UI_STATS_DIALOG_MODE mode, const int32_t level_num,
    const GAME_FLOW_LEVEL_TYPE level_type)
{
    UI_STATS_DIALOG *const self = Memory_Alloc(sizeof(UI_STATS_DIALOG));
    self->vtable = (UI_WIDGET_VTABLE) {
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->mode = mode;
    self->level_num = level_num;
    self->level_type = level_type;

    self->row_count = 0;
    self->rows = NULL;
    self->stack = UI_Stack_Create(
        UI_STACK_LAYOUT_VERTICAL, UI_STACK_AUTO_SIZE, UI_STACK_AUTO_SIZE);
    UI_Stack_SetHAlign(self->stack, UI_STACK_H_ALIGN_CENTER);

    self->listener =
        UI_Events_Subscribe("layout_update", NULL, M_HandleLayoutUpdate, self);

    switch (mode) {
    case UI_STATS_DIALOG_MODE_LEVEL:
        self->title = UI_Label_Create(
            g_GameFlow.levels[self->level_num].level_title, UI_LABEL_AUTO_SIZE,
            ROW_HEIGHT);
        UI_Stack_AddChild(self->stack, self->title);
        M_AddLevelStatsRows(self);
        break;

    case UI_STATS_DIALOG_MODE_FINAL:
        self->title = UI_Label_Create(
            level_type == GFL_BONUS ? GS(STATS_BONUS_STATISTICS)
                                    : GS(STATS_FINAL_STATISTICS),
            UI_LABEL_AUTO_SIZE, ROW_HEIGHT);
        UI_Stack_AddChild(self->stack, self->title);
        M_AddFinalStatsRows(self, level_type);
        break;
    }

    M_DoLayout(self);

    return (UI_WIDGET *)self;
}
