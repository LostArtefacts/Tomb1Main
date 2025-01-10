#include "game/ui/widgets/requester.h"

#include "game/game_string.h"
#include "game/input.h"
#include "game/text.h"
#include "game/ui/common.h"
#include "game/ui/widgets/frame.h"
#include "game/ui/widgets/label.h"
#include "game/ui/widgets/stack.h"
#include "game/ui/widgets/window.h"
#include "log.h"
#include "memory.h"

#include <stdio.h>

typedef struct {
    void *user_data;
    UI_WIDGET *frame;
    UI_WIDGET *stack;
    UI_WIDGET *left_label;
    UI_WIDGET *right_label;
} M_ROW;

typedef struct {
    UI_WIDGET_VTABLE vtable;
    UI_WIDGET *window;
    UI_WIDGET *outer_stack;
    UI_REQUESTER_SETTINGS settings;

    bool is_confirmed;
    int32_t selected_row_offset;
    int32_t visible_row_offset;
    int32_t row_count;
    M_ROW *rows;

    int32_t selection_margin;
    int32_t selection_padding;

    int32_t listener;
} UI_REQUESTER;

static void M_ClearRows(UI_REQUESTER *self);
static M_ROW *M_AddRow(
    UI_REQUESTER *self, const char *left_text, const char *right_text,
    void *user_data);
static void M_DoLayout(UI_REQUESTER *self);
static void M_HandleCanvasResize(const EVENT *event, void *data);

static int32_t M_GetWidth(const UI_REQUESTER *self);
static int32_t M_GetHeight(const UI_REQUESTER *self);
static void M_SetPosition(UI_REQUESTER *self, int32_t x, int32_t y);
static void M_Control(UI_REQUESTER *self);
static void M_Draw(UI_REQUESTER *self);
static void M_Free(UI_REQUESTER *self);

static void M_ClearRows(UI_REQUESTER *const self)
{
    for (int32_t i = 0; i < self->row_count; i++) {
        self->rows[i].left_label->free(self->rows[i].left_label);
        if (self->rows[i].right_label != NULL) {
            self->rows[i].right_label->free(self->rows[i].right_label);
        }
        self->rows[i].frame->free(self->rows[i].frame);
        self->rows[i].stack->free(self->rows[i].stack);
    }
    UI_Stack_ClearChildren(self->outer_stack);
    self->visible_row_offset = 0;
    self->row_count = 0;
    self->selected_row_offset = -1;
    self->is_confirmed = false;
}

static M_ROW *M_AddRow(
    UI_REQUESTER *const self, const char *const left_text,
    const char *const right_text, void *const user_data)
{
    self->row_count++;
    self->rows = Memory_Realloc(self->rows, sizeof(M_ROW) * self->row_count);
    M_ROW *const row = &self->rows[self->row_count - 1];

    row->stack = UI_Stack_Create(
        UI_STACK_LAYOUT_HORIZONTAL, self->settings.width, UI_STACK_AUTO_SIZE);
    UI_Stack_SetHAlign(row->stack, UI_STACK_H_ALIGN_DISTRIBUTE);

    row->frame = UI_Frame_Create(row->stack, 0, 0);
    UI_Frame_SetFrameVisible(row->frame, false);

    row->left_label = UI_Label_Create(
        left_text, UI_LABEL_AUTO_SIZE, self->settings.row_height);
    UI_Stack_AddChild(row->stack, row->left_label);

    if (right_text != NULL) {
        row->right_label = UI_Label_Create(
            right_text, UI_LABEL_AUTO_SIZE, self->settings.row_height);
        UI_Stack_AddChild(row->stack, row->right_label);
    } else {
        row->right_label = NULL;
        UI_Stack_SetHAlign(row->stack, UI_STACK_H_ALIGN_CENTER);
    }

    row->user_data = user_data;

    for (int32_t y = 0; y < self->row_count; y++) {
        self->rows[y].stack->is_hidden = y < self->visible_row_offset
            || y >= self->visible_row_offset + self->settings.visible_rows;
    }
    if (self->settings.is_selectable && self->selected_row_offset == -1) {
        self->selected_row_offset = 0;
        UI_Frame_SetFrameVisible(
            self->rows[self->selected_row_offset].frame, true);
    }

    UI_Stack_AddChild(self->outer_stack, row->frame);
    return row;
}

static void M_DoLayout(UI_REQUESTER *const self)
{
    UI_HandleLayoutChange();
}

static void M_HandleCanvasResize(const EVENT *event, void *data)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)data;
    M_DoLayout(self);
}

static int32_t M_GetWidth(const UI_REQUESTER *const self)
{
    return self->window->get_width(self->window);
}

static int32_t M_GetHeight(const UI_REQUESTER *const self)
{
    return self->window->get_height(self->window);
}

static void M_SetPosition(
    UI_REQUESTER *const self, const int32_t x, const int32_t y)
{
    self->window->set_position(self->window, x, y);
}

static void M_Control(UI_REQUESTER *const self)
{
    if (self->window->control != NULL) {
        self->window->control(self->window);
    }

    bool update = false;
    if (g_InputDB.menu_down) {
        if (self->visible_row_offset + self->settings.visible_rows
            < self->row_count) {
            self->rows[self->visible_row_offset].stack->is_hidden = true;
            self->rows[self->visible_row_offset + self->settings.visible_rows]
                .stack->is_hidden = false;
            self->visible_row_offset++;
            update = true;
        }
    } else if (g_InputDB.menu_up) {
        if (self->visible_row_offset > 0) {
            self->rows
                [self->visible_row_offset + self->settings.visible_rows - 1]
                    .stack->is_hidden = true;
            self->rows[self->visible_row_offset - 1].stack->is_hidden = false;
            self->visible_row_offset--;
            update = true;
        }
    }

    if (self->settings.is_selectable) {
        if (g_InputDB.menu_down
            && self->selected_row_offset + 1 < self->row_count) {
            if (self->selected_row_offset != -1) {
                UI_Frame_SetFrameVisible(
                    self->rows[self->selected_row_offset].frame, false);
            }
            self->selected_row_offset++;
            UI_Frame_SetFrameVisible(
                self->rows[self->selected_row_offset].frame, true);
            update = true;
        } else if (g_InputDB.menu_up && self->selected_row_offset > 0) {
            if (self->selected_row_offset != -1) {
                UI_Frame_SetFrameVisible(
                    self->rows[self->selected_row_offset].frame, false);
            }
            self->selected_row_offset--;
            UI_Frame_SetFrameVisible(
                self->rows[self->selected_row_offset].frame, true);
            update = true;
        }
        if (g_InputDB.menu_confirm) {
            self->is_confirmed = true;
        }
    }

    if (update) {
        M_DoLayout(self);
    }
}

static void M_Draw(UI_REQUESTER *const self)
{
    if (self->window->draw != NULL) {
        self->window->draw(self->window);
    }
}

static void M_Free(UI_REQUESTER *const self)
{
    M_ClearRows(self);
    self->outer_stack->free(self->outer_stack);
    self->window->free(self->window);
    UI_Events_Unsubscribe(self->listener);
    Memory_Free(self);
}

UI_WIDGET *UI_Requester_Create(UI_REQUESTER_SETTINGS settings)
{
    UI_REQUESTER *const self = Memory_Alloc(sizeof(UI_REQUESTER));
    if (settings.row_height == 0) {
        settings.row_height = 18;
    }
    if (settings.width == 0) {
        settings.width = UI_STACK_AUTO_SIZE;
    }

    self->vtable = (UI_WIDGET_VTABLE) {
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->settings = settings;
    self->outer_stack = UI_Stack_Create(
        UI_STACK_LAYOUT_VERTICAL, UI_STACK_AUTO_SIZE,
        settings.row_height * self->settings.visible_rows);
    UI_Stack_SetHAlign(self->outer_stack, UI_STACK_H_ALIGN_CENTER);

    self->window = UI_Window_Create(self->outer_stack, 8, 8, 8, 8);

    self->selected_row_offset = -1;
    self->listener =
        UI_Events_Subscribe("canvas_resize", NULL, M_HandleCanvasResize, self);

    M_DoLayout(self);
    return (UI_WIDGET *)self;
}

int32_t UI_Requester_GetSelectedRow(UI_WIDGET *const widget)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    return self->is_confirmed ? self->selected_row_offset : -1;
}

void UI_Requester_SetTitle(UI_WIDGET *const widget, const char *const title)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    UI_Window_SetTitle(self->window, title);
    M_DoLayout(self);
}

void UI_Requester_ClearRows(UI_WIDGET *const widget)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    M_ClearRows(self);
    M_DoLayout(self);
}

void UI_Requester_AddRowLR(
    UI_WIDGET *const widget, const char *const text_l, const char *const text_r,
    void *const user_data)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    M_AddRow(self, text_l, text_r, user_data);
    M_DoLayout(self);
}

void UI_Requester_AddRowC(
    UI_WIDGET *const widget, const char *const text, void *const user_data)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    M_AddRow(self, text, NULL, user_data);
    M_DoLayout(self);
}

void *UI_Requester_GetRowUserData(UI_WIDGET *const widget, const int32_t idx)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    if (idx >= self->row_count || idx < 0) {
        return NULL;
    }
    return self->rows[idx].user_data;
}

int32_t UI_Requester_GetRowCount(UI_WIDGET *const widget)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    return self->row_count;
}

void UI_Requester_ChangeRowLR(
    UI_WIDGET *widget, int32_t idx, const char *text_l, const char *text_r,
    void *user_data)
{
    UI_REQUESTER *const self = (UI_REQUESTER *)widget;
    if (idx >= self->row_count || idx < 0) {
        return;
    }
    if (self->rows[idx].left_label != NULL && text_l != NULL) {
        UI_Label_ChangeText(self->rows[idx].left_label, text_l);
    }
    if (self->rows[idx].right_label != NULL && text_r != NULL) {
        UI_Label_ChangeText(self->rows[idx].right_label, text_r);
    }
    self->rows[idx].user_data = user_data;
    M_DoLayout(self);
}
