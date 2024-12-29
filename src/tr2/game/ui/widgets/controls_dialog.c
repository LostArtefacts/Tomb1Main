#include "game/ui/widgets/controls_dialog.h"

#include "game/game_string.h"
#include "game/ui/widgets/controls_backend_selector.h"
#include "game/ui/widgets/controls_column.h"
#include "game/ui/widgets/controls_layout_editor.h"

#include <libtrx/game/ui/common.h>
#include <libtrx/game/ui/widgets/window.h>
#include <libtrx/memory.h>

typedef struct {
    UI_WIDGET_VTABLE vtable;
    UI_CONTROLS_CONTROLLER *controller;
    UI_WIDGET *window;
    UI_WIDGET *backend_selector;
    UI_WIDGET *layout_editor;
    int32_t listener;
} UI_CONTROLS_DIALOG;

static void M_DoLayout(UI_CONTROLS_DIALOG *self);
static void M_HandleCanvasResize(const EVENT *event, void *data);
static int32_t M_GetWidth(const UI_CONTROLS_DIALOG *self);
static int32_t M_GetHeight(const UI_CONTROLS_DIALOG *self);
static void M_SetPosition(UI_CONTROLS_DIALOG *self, int32_t x, int32_t y);
static void M_Control(UI_CONTROLS_DIALOG *self);
static void M_Draw(UI_CONTROLS_DIALOG *self);
static void M_Free(UI_CONTROLS_DIALOG *self);

static void M_DoLayout(UI_CONTROLS_DIALOG *const self)
{
    M_SetPosition(
        self, (UI_GetCanvasWidth() - M_GetWidth(self)) / 2,
        (UI_GetCanvasHeight() - M_GetHeight(self)) * 2 / 3);
}

static void M_HandleCanvasResize(const EVENT *event, void *data)
{
    UI_CONTROLS_DIALOG *const self = (UI_CONTROLS_DIALOG *)data;
    M_DoLayout(self);
}

static int32_t M_GetWidth(const UI_CONTROLS_DIALOG *const self)
{
    return self->window->get_width(self->window);
}

static int32_t M_GetHeight(const UI_CONTROLS_DIALOG *const self)
{
    return self->window->get_height(self->window);
}

static void M_SetPosition(
    UI_CONTROLS_DIALOG *const self, const int32_t x, const int32_t y)
{
    return self->window->set_position(self->window, x, y);
}

static void M_Control(UI_CONTROLS_DIALOG *const self)
{
    // Trigger the UI updates only if anything has changed.
    if (UI_ControlsController_Control(self->controller)) {
        // Set the root widget - backend selector modal or the inputs modal
        if (self->controller->state == UI_CONTROLS_STATE_NAVIGATE_BACKEND
            || self->controller->state == UI_CONTROLS_STATE_EXIT) {
            UI_Window_SetTitle(self->window, GS(CONTROL_CUSTOMIZE));
            UI_Window_SetRootWidget(self->window, self->backend_selector);
        } else {
            UI_Window_SetTitle(self->window, NULL);
            UI_Window_SetRootWidget(self->window, self->layout_editor);
        }
        M_DoLayout(self);

        if (self->window->control != NULL) {
            self->window->control(self->window);
        }
    }
}

static void M_Draw(UI_CONTROLS_DIALOG *const self)
{
    if (self->window->draw != NULL) {
        self->window->draw(self->window);
    }
}

static void M_Free(UI_CONTROLS_DIALOG *const self)
{
    self->layout_editor->free(self->layout_editor);
    self->backend_selector->free(self->backend_selector);
    self->window->free(self->window);
    UI_Events_Unsubscribe(self->listener);
    Memory_Free(self);
}

UI_WIDGET *UI_ControlsDialog_Create(UI_CONTROLS_CONTROLLER *const controller)
{
    UI_CONTROLS_DIALOG *const self = Memory_Alloc(sizeof(UI_CONTROLS_DIALOG));
    self->vtable = (UI_WIDGET_VTABLE) {
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->controller = controller;

    self->layout_editor = UI_ControlsLayoutEditor_Create(self->controller);
    self->backend_selector =
        UI_ControlsBackendSelector_Create(self->controller);
    self->window = UI_Window_Create(self->backend_selector, 5, 5, 10, 5);

    UI_Window_SetTitle(self->window, GS(CONTROL_CUSTOMIZE));

    self->listener =
        UI_Events_Subscribe("canvas_resize", NULL, M_HandleCanvasResize, self);

    M_DoLayout(self);
    return (UI_WIDGET *)self;
}
