#include "game/text.h"

#include <libtrx/game/ui/widgets/label.h>
#include <libtrx/game/ui/widgets/window.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>

typedef struct {
    UI_WIDGET_VTABLE vtable;
    TEXTSTRING *frame;
    UI_WIDGET *root;
    UI_WIDGET *title_label;
    struct {
        int32_t left;
        int32_t right;
        int32_t top;
        int32_t bottom;
    } border;
    int32_t title_margin;
} UI_WINDOW;

static int32_t M_GetWidth(const UI_WINDOW *self);
static int32_t M_GetHeight(const UI_WINDOW *self);
static void M_SetPosition(UI_WINDOW *self, int32_t x, int32_t y);
static void M_Control(UI_WINDOW *self);
static void M_Draw(UI_WINDOW *self);
static void M_Free(UI_WINDOW *self);

static int32_t M_GetWidth(const UI_WINDOW *const self)
{
    const int32_t title_width = self->title_label != NULL
        ? self->title_label->get_width(self->title_label)
            + 2 * self->title_margin
        : 0;
    const int32_t root_width = self->root->get_width(self->root)
        + self->border.left + self->border.right;
    return MAX(title_width, root_width);
}

static int32_t M_GetHeight(const UI_WINDOW *const self)
{
    const int32_t title_height = self->title_label != NULL
        ? self->title_label->get_height(self->title_label)
            + 2 * self->title_margin
        : 0;
    const int32_t root_height = self->root->get_height(self->root)
        + self->border.top + self->border.bottom;
    return title_height + root_height;
}

static void M_SetPosition(
    UI_WINDOW *const self, const int32_t x, const int32_t y)
{
    if (self->title_label != NULL) {
        self->title_label->set_position(
            self->title_label, x + self->title_margin, y + self->title_margin);
        self->root->set_position(
            self->root,
            x + self->border.left
                + (self->title_label->get_width(self->title_label)
                   - self->root->get_width(self->root))
                    / 2,
            y + self->title_margin
                + self->title_label->get_height(self->title_label)
                + self->title_margin + self->border.top);
    } else {
        self->root->set_position(
            self->root, x + self->border.left, y + self->border.top);
    }

    Text_SetPos(self->frame, x, y + TEXT_HEIGHT);

    const int32_t w = M_GetWidth(self);
    const int32_t h = M_GetHeight(self);
    Text_AddBackground(self->frame, w, h, w / 2, 0, TS_BACKGROUND);
    Text_AddOutline(self->frame, true, INV_COLOR_BLUE, NULL);
}

static void M_Control(UI_WINDOW *const self)
{
    if (self->root->control != NULL) {
        self->root->control(self->root);
    }
}

static void M_Draw(UI_WINDOW *const self)
{
    if (self->root->draw != NULL) {
        self->root->draw(self->root);
    }
    if (self->title_label != NULL) {
        self->title_label->draw(self->title_label);
    }
    Text_DrawText(self->frame);
}

static void M_Free(UI_WINDOW *const self)
{
    if (self->title_label != NULL) {
        self->title_label->free(self->title_label);
    }
    Text_Remove(self->frame);
    Memory_Free(self);
}

UI_WIDGET *UI_Window_Create(
    UI_WIDGET *const root, const int32_t border_top, const int32_t border_right,
    const int32_t border_bottom, const int32_t border_left)
{
    UI_WINDOW *const self = Memory_Alloc(sizeof(UI_WINDOW));
    self->vtable = (UI_WIDGET_VTABLE) {
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->root = root;
    self->border.top = border_top;
    self->border.right = border_right;
    self->border.bottom = border_bottom;
    self->border.left = border_left;
    self->title_margin = 2;

    self->frame = Text_Create(0, 0, "");
    self->frame->pos.z = 32;
    self->frame->flags.manual_draw = 1;

    return (UI_WIDGET *)self;
}

void UI_Window_SetTitle(UI_WIDGET *const widget, const char *const text)
{
    UI_WINDOW *const self = (UI_WINDOW *)widget;
    if (self->title_label != NULL) {
        self->title_label->free(self->title_label);
        self->title_label = NULL;
    }
    if (text != NULL) {
        self->title_label =
            UI_Label_Create(text, UI_LABEL_AUTO_SIZE, UI_LABEL_AUTO_SIZE);
        UI_Label_AddFrame(self->title_label);
    }
}

void UI_Window_SetRootWidget(UI_WIDGET *const widget, UI_WIDGET *const root)
{
    UI_WINDOW *const self = (UI_WINDOW *)widget;
    self->root = root;
}
