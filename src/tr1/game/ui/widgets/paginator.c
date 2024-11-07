#include "game/ui/widgets/paginator.h"

#include "game/game_string.h"
#include "game/input.h"
#include "game/screen.h"
#include "game/sound.h"
#include "game/text.h"

#include <libtrx/game/ui/common.h>
#include <libtrx/game/ui/widgets/label.h>
#include <libtrx/game/ui/widgets/spacer.h>
#include <libtrx/game/ui/widgets/stack.h>
#include <libtrx/game/ui/widgets/window.h>
#include <libtrx/memory.h>
#include <libtrx/strings/common.h>

#include <stdio.h>

#define TITLE_MARGIN 5
#define WINDOW_MARGIN 10
#define DIALOG_PADDING 5
#define PADDING_SCALED (3.5 * (DIALOG_PADDING + WINDOW_MARGIN))

typedef struct {
    UI_WIDGET_VTABLE vtable;
    UI_WIDGET *window;
    UI_WIDGET *outer_stack;
    UI_WIDGET *bottom_stack;
    UI_WIDGET *title;
    UI_WIDGET *top_spacer;
    UI_WIDGET *text;
    UI_WIDGET *bottom_spacer;
    UI_WIDGET *left_arrow;
    UI_WIDGET *right_arrow;
    UI_WIDGET *right_arrow_spacer;
    UI_WIDGET *page_label;
    int32_t current_page;
    VECTOR *page_content;
} UI_PAGINATOR;

static void M_DoLayout(UI_PAGINATOR *self);
static int32_t M_GetWidth(const UI_PAGINATOR *self);
static int32_t M_GetHeight(const UI_PAGINATOR *self);
static void M_SetPosition(UI_PAGINATOR *self, int32_t x, int32_t y);
static bool M_SelectPage(UI_PAGINATOR *const self, int32_t new_page);
static void M_Control(UI_PAGINATOR *self);
static void M_Draw(UI_PAGINATOR *self);
static void M_Free(UI_PAGINATOR *self);

static void M_DoLayout(UI_PAGINATOR *const self)
{
    M_SetPosition(
        self, (Screen_GetResWidthDownscaled(RSR_TEXT) - M_GetWidth(self)) / 2.0,
        (Screen_GetResHeightDownscaled(RSR_TEXT) - M_GetHeight(self)) / 2.0);
}

static int32_t M_GetWidth(const UI_PAGINATOR *const self)
{
    return self->window->get_width(self->window);
}

static int32_t M_GetHeight(const UI_PAGINATOR *const self)
{
    return self->window->get_height(self->window);
}

static void M_SetPosition(UI_PAGINATOR *const self, int32_t x, int32_t y)
{
    return self->window->set_position(self->window, x, y);
}

static bool M_SelectPage(UI_PAGINATOR *const self, const int32_t new_page)
{
    if (new_page == self->current_page || new_page < 0
        || new_page >= self->page_content->count) {
        return false;
    }

    self->current_page = new_page;
    UI_Label_ChangeText(
        self->text,
        *(char **)Vector_Get(self->page_content, self->current_page));

    char page_indicator[100];
    sprintf(
        page_indicator, GS(PAGINATION_NAV), self->current_page + 1,
        self->page_content->count);
    UI_Label_ChangeText(self->page_label, page_indicator);
    UI_Label_SetVisible(self->left_arrow, self->current_page > 0);
    UI_Label_SetVisible(
        self->right_arrow, self->current_page < self->page_content->count - 1);

    return true;
}

static void M_Control(UI_PAGINATOR *const self)
{
    const int32_t page_shift = g_InputDB.left ? -1 : (g_InputDB.right ? 1 : 0);
    if (M_SelectPage(self, self->current_page + page_shift)) {
        Sound_Effect(SFX_MENU_PASSPORT, NULL, SPM_ALWAYS);
    }

    if (self->window->control != NULL) {
        self->window->control(self->window);
    }
}

static void M_Draw(UI_PAGINATOR *const self)
{
    if (self->window->draw != NULL) {
        self->window->draw(self->window);
    }
}

static void M_Free(UI_PAGINATOR *const self)
{
    for (int32_t i = self->page_content->count - 1; i >= 0; i--) {
        char *const page = *(char **)Vector_Get(self->page_content, i);
        Memory_Free(page);
    }
    Vector_Free(self->page_content);

    self->text->free(self->text);
    self->top_spacer->free(self->top_spacer);
    self->title->free(self->title);
    if (self->bottom_stack != NULL) {
        self->bottom_spacer->free(self->bottom_spacer);
        self->left_arrow->free(self->left_arrow);
        self->right_arrow->free(self->right_arrow);
        self->right_arrow_spacer->free(self->right_arrow_spacer);
        self->page_label->free(self->page_label);
        self->bottom_stack->free(self->bottom_stack);
    }
    self->outer_stack->free(self->outer_stack);
    self->window->free(self->window);
    Memory_Free(self);
}

UI_WIDGET *UI_Paginator_Create(
    const char *const title, const char *const text, const int32_t max_lines)
{
    UI_PAGINATOR *const self = Memory_Alloc(sizeof(UI_PAGINATOR));
    self->vtable = (UI_WIDGET_VTABLE) {
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->outer_stack = UI_Stack_Create(
        UI_STACK_LAYOUT_VERTICAL, UI_STACK_AUTO_SIZE, UI_STACK_AUTO_SIZE);

    const char *upper_title = String_ToUpper(title);
    self->title =
        UI_Label_Create(upper_title, UI_LABEL_AUTO_SIZE, TEXT_HEIGHT_FIXED);
    Memory_FreePointer(&upper_title);
    UI_Stack_AddChild(self->outer_stack, self->title);

    self->top_spacer = UI_Spacer_Create(TITLE_MARGIN, TITLE_MARGIN);
    UI_Stack_AddChild(self->outer_stack, self->top_spacer);

    const char *wrapped =
        String_WordWrap(text, Text_GetMaxLineLength() - PADDING_SCALED);
    self->page_content = String_Paginate(wrapped, max_lines);
    self->current_page = 0;
    Memory_FreePointer(&wrapped);

    self->text = UI_Label_Create(
        *(char **)Vector_Get(self->page_content, 0), UI_LABEL_AUTO_SIZE,
        UI_LABEL_AUTO_SIZE);
    UI_Stack_AddChild(self->outer_stack, self->text);

    if (self->page_content->count > 1) {
        self->bottom_spacer = UI_Spacer_Create(TITLE_MARGIN, TITLE_MARGIN * 3);
        UI_Stack_AddChild(self->outer_stack, self->bottom_spacer);

        self->bottom_stack = UI_Stack_Create(
            UI_STACK_LAYOUT_HORIZONTAL, UI_STACK_AUTO_SIZE, UI_STACK_AUTO_SIZE);
        UI_Stack_AddChild(self->outer_stack, self->bottom_stack);

        self->left_arrow =
            UI_Label_Create("\\{button left}", 22, TEXT_HEIGHT_FIXED);
        self->right_arrow =
            UI_Label_Create("\\{button right}", 16, TEXT_HEIGHT_FIXED);
        self->right_arrow_spacer = UI_Spacer_Create(6, TEXT_HEIGHT_FIXED);
        self->page_label =
            UI_Label_Create("", UI_LABEL_AUTO_SIZE, UI_LABEL_AUTO_SIZE);
        UI_Stack_AddChild(self->bottom_stack, self->left_arrow);
        UI_Stack_AddChild(self->bottom_stack, self->page_label);
        UI_Stack_AddChild(self->bottom_stack, self->right_arrow_spacer);
        UI_Stack_AddChild(self->bottom_stack, self->right_arrow);

        UI_Stack_SetHAlign(self->bottom_stack, UI_STACK_H_ALIGN_RIGHT);
    }

    self->window = UI_Window_Create(
        self->outer_stack, DIALOG_PADDING, DIALOG_PADDING, DIALOG_PADDING * 2,
        DIALOG_PADDING);

    // Ensure minimum width for page navigation as text content may be empty.
    int32_t max_width =
        MAX(self->page_content->count == 1 ? 0 : 100,
            UI_Label_MeasureTextWidth(self->title));
    int32_t max_nav_width = 0;
    int32_t max_height = 0;
    for (int32_t i = 0; i < self->page_content->count; i++) {
        M_SelectPage(self, i);
        max_width = MAX(max_width, UI_Label_MeasureTextWidth(self->text));
        max_height = MAX(max_height, UI_Label_MeasureTextHeight(self->text));
        if (self->bottom_stack != NULL) {
            max_nav_width =
                MAX(max_nav_width, UI_Label_MeasureTextWidth(self->page_label));
        }
    }

    UI_Label_SetSize(self->text, max_width, max_height);
    if (self->bottom_stack != NULL) {
        UI_Stack_SetSize(self->bottom_stack, max_width, UI_STACK_AUTO_SIZE);
        UI_Label_SetSize(self->page_label, max_nav_width, UI_LABEL_AUTO_SIZE);
    }

    M_SelectPage(self, 0);
    M_DoLayout(self);

    return (UI_WIDGET *)self;
}
