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
#include <libtrx/game/ui/widgets/window.h>
#include <libtrx/memory.h>

#include <stdio.h>
#include <string.h>

#define ROW_HEIGHT_BARE 30
#define ROW_HEIGHT_BORDERED 18

typedef enum {
    M_ROW_KILLS,
    M_ROW_PICKUPS,
    M_ROW_SECRETS,
    M_ROW_DEATHS,
    M_ROW_TIMER,
} M_ROW_ROLE;

typedef struct {
    M_ROW_ROLE role;
    UI_WIDGET *stack;
    UI_WIDGET *key_label;
    UI_WIDGET *value_label;
} M_ROW;

typedef struct {
    UI_WIDGET_VTABLE vtable;
    UI_STATS_DIALOG_ARGS args;
    GAME_FLOW_LEVEL_TYPE level_type;
    int32_t listener;

    int32_t row_count;
    UI_WIDGET *title;
    UI_WIDGET *stack;
    UI_WIDGET *window;
    UI_WIDGET *root; // just a pointer to either stack or window
    M_ROW *rows;
} UI_STATS_DIALOG;

static void M_FormatTime(char *out, int32_t total_frames);
static const char *M_GetDialogTitle(UI_STATS_DIALOG *self);
static void M_AddRow(
    UI_STATS_DIALOG *self, M_ROW_ROLE role, const char *key, const char *value);
static void M_AddRowFromRole(
    UI_STATS_DIALOG *self, M_ROW_ROLE role, const STATS_COMMON *stats);
static void M_AddCommonRows(UI_STATS_DIALOG *self, const STATS_COMMON *stats);
static void M_AddLevelStatsRows(UI_STATS_DIALOG *self);
static void M_AddFinalStatsRows(UI_STATS_DIALOG *self);
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
    if (hours != 0) {
        sprintf(out, "%d:%02d:%02d", hours, minutes, seconds);
    } else {
        sprintf(out, "%d:%02d", minutes, seconds);
    }
}

static const char *M_GetDialogTitle(UI_STATS_DIALOG *const self)
{
    switch (self->args.mode) {
    case UI_STATS_DIALOG_MODE_LEVEL:
        return GameFlow_GetLevelTitle(self->args.level_num);

    case UI_STATS_DIALOG_MODE_FINAL:
        return self->level_type == GFL_BONUS ? GS(STATS_BONUS_STATISTICS)
                                             : GS(STATS_FINAL_STATISTICS);
    }

    return NULL;
}

static void M_AddRow(
    UI_STATS_DIALOG *const self, const M_ROW_ROLE role, const char *const key,
    const char *const value)
{
    self->row_count++;
    self->rows = Memory_Realloc(self->rows, sizeof(M_ROW) * self->row_count);
    M_ROW *const row = &self->rows[self->row_count - 1];
    row->role = role;

    // create a stack
    int32_t row_height;
    if (self->args.style == UI_STATS_DIALOG_STYLE_BARE) {
        row_height = ROW_HEIGHT_BARE;
        row->stack = UI_Stack_Create(
            UI_STACK_LAYOUT_HORIZONTAL, UI_STACK_AUTO_SIZE, row_height);
    } else {
        row_height = ROW_HEIGHT_BORDERED;
        row->stack =
            UI_Stack_Create(UI_STACK_LAYOUT_HORIZONTAL, 200, row_height);
        UI_Stack_SetHAlign(row->stack, UI_STACK_H_ALIGN_DISTRIBUTE);
    }

    // create a key label; append space for the bare style
    char key2[strlen(key) + 2];
    sprintf(
        key2, self->args.style == UI_STATS_DIALOG_STYLE_BARE ? "%s " : "%s",
        key);
    row->key_label = UI_Label_Create(key2, UI_LABEL_AUTO_SIZE, row_height);

    // create a value label
    row->value_label = UI_Label_Create(value, UI_LABEL_AUTO_SIZE, row_height);

    UI_Stack_AddChild(row->stack, row->key_label);
    UI_Stack_AddChild(row->stack, row->value_label);
    UI_Stack_AddChild(self->stack, row->stack);
}

static void M_AddRowFromRole(
    UI_STATS_DIALOG *const self, const M_ROW_ROLE role,
    const STATS_COMMON *const stats)
{
    char buf[50];
    const char *const num_fmt = g_Config.gameplay.enable_detailed_stats
        ? GS(STATS_DETAIL_FMT)
        : GS(STATS_BASIC_FMT);

    switch (role) {
    case M_ROW_KILLS:
        sprintf(buf, num_fmt, stats->kill_count, stats->max_kill_count);
        M_AddRow(self, role, GS(STATS_KILLS), buf);
        break;

    case M_ROW_PICKUPS:
        sprintf(buf, num_fmt, stats->pickup_count, stats->max_pickup_count);
        M_AddRow(self, role, GS(STATS_PICKUPS), buf);
        break;

    case M_ROW_SECRETS:
        sprintf(
            buf, GS(STATS_DETAIL_FMT), stats->secret_count,
            stats->max_secret_count);
        M_AddRow(self, role, GS(STATS_SECRETS), buf);
        break;

    case M_ROW_DEATHS:
        sprintf(buf, "%d", stats->death_count);
        M_AddRow(self, role, GS(STATS_DEATHS), buf);
        break;

    case M_ROW_TIMER:
        M_FormatTime(buf, stats->timer);
        M_AddRow(self, role, GS(STATS_TIME_TAKEN), buf);
        break;

    default:
        break;
    }
}

static void M_AddCommonRows(
    UI_STATS_DIALOG *const self, const STATS_COMMON *const stats)
{
    M_AddRowFromRole(self, M_ROW_KILLS, stats);
    M_AddRowFromRole(self, M_ROW_PICKUPS, stats);
    M_AddRowFromRole(self, M_ROW_SECRETS, stats);
    if (g_Config.gameplay.enable_deaths_counter
        && g_GameInfo.death_counter_supported) {
        M_AddRowFromRole(self, M_ROW_DEATHS, stats);
    }
    M_AddRowFromRole(self, M_ROW_TIMER, stats);
}

static void M_AddLevelStatsRows(UI_STATS_DIALOG *const self)
{
    const STATS_COMMON *stats =
        (STATS_COMMON *)&g_GameInfo.current[self->args.level_num].stats;
    M_AddCommonRows(self, stats);
}

static void M_AddFinalStatsRows(UI_STATS_DIALOG *const self)
{
    FINAL_STATS final_stats;
    Stats_ComputeFinal(self->level_type, &final_stats);
    M_AddCommonRows(self, (STATS_COMMON *)&final_stats);
}

static void M_UpdateTimerRow(UI_STATS_DIALOG *const self)
{
    if (self->args.mode != UI_STATS_DIALOG_MODE_LEVEL) {
        return;
    }

    for (int32_t i = 0; i < self->row_count; i++) {
        if (self->rows[i].role != M_ROW_TIMER) {
            continue;
        }
        char buf[50];
        M_FormatTime(buf, g_GameInfo.current[self->args.level_num].stats.timer);
        UI_Label_ChangeText(self->rows[i].value_label, buf);
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
    return self->root->get_width(self->root);
}

static int32_t M_GetHeight(const UI_STATS_DIALOG *const self)
{
    return self->root->get_height(self->root);
}

static void M_SetPosition(
    UI_STATS_DIALOG *const self, const int32_t x, const int32_t y)
{
    self->root->set_position(self->root, x, y);
}

static void M_Control(UI_STATS_DIALOG *const self)
{
    if (self->root->control != NULL) {
        self->root->control(self->root);
    }
    M_UpdateTimerRow(self);
}

static void M_Draw(UI_STATS_DIALOG *const self)
{
    if (self->root->draw != NULL) {
        self->root->draw(self->root);
    }
}

static void M_Free(UI_STATS_DIALOG *const self)
{
    if (self->title != NULL) {
        self->title->free(self->title);
    }
    for (int32_t i = 0; i < self->row_count; i++) {
        self->rows[i].key_label->free(self->rows[i].key_label);
        self->rows[i].value_label->free(self->rows[i].value_label);
        self->rows[i].stack->free(self->rows[i].stack);
    }
    if (self->window != NULL) {
        self->window->free(self->window);
    }
    self->stack->free(self->stack);
    UI_Events_Unsubscribe(self->listener);
    Memory_Free(self);
}

UI_WIDGET *UI_StatsDialog_Create(const UI_STATS_DIALOG_ARGS args)
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

    self->args = args;
    self->level_type = g_GameFlow.levels[self->args.level_num].level_type;

    self->row_count = 0;
    self->rows = NULL;
    self->stack = UI_Stack_Create(
        UI_STACK_LAYOUT_VERTICAL, UI_STACK_AUTO_SIZE, UI_STACK_AUTO_SIZE);
    UI_Stack_SetHAlign(self->stack, UI_STACK_H_ALIGN_CENTER);

    self->listener =
        UI_Events_Subscribe("layout_update", NULL, M_HandleLayoutUpdate, self);

    const char *title = M_GetDialogTitle(self);
    switch (self->args.style) {
    case UI_STATS_DIALOG_STYLE_BARE:
        if (title != NULL) {
            self->title =
                UI_Label_Create(title, UI_LABEL_AUTO_SIZE, ROW_HEIGHT_BARE);
            UI_Stack_AddChild(self->stack, self->title);
        }
        self->root = self->stack;
        break;

    case UI_STATS_DIALOG_STYLE_BORDERED:
        self->window = UI_Window_Create(self->stack, 8, 8, 8, 8);
        UI_Window_SetTitle(self->window, title);
        self->root = self->window;
        break;
    }

    if (self->args.mode == UI_STATS_DIALOG_MODE_LEVEL) {
        M_AddLevelStatsRows(self);
    } else if (self->args.mode == UI_STATS_DIALOG_MODE_FINAL) {
        M_AddFinalStatsRows(self);
    }

    M_DoLayout(self);
    return (UI_WIDGET *)self;
}
