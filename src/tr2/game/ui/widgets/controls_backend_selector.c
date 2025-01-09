#include "game/ui/widgets/controls_backend_selector.h"

#include "game/game_string.h"

#include <libtrx/game/ui/widgets/label.h>
#include <libtrx/game/ui/widgets/stack.h>
#include <libtrx/memory.h>

typedef struct {
    UI_WIDGET_VTABLE vtable;
    UI_WIDGET *keyboard_label;
    UI_WIDGET *controller_label;
    UI_WIDGET *container;
    UI_CONTROLS_CONTROLLER *controller;
} UI_CONTROLS_BACKEND_SELECTOR;

static void M_SyncOutlines(UI_CONTROLS_BACKEND_SELECTOR *self);

static void M_UpdateText(UI_CONTROLS_BACKEND_SELECTOR *self);
static int32_t M_GetWidth(const UI_CONTROLS_BACKEND_SELECTOR *self);
static int32_t M_GetHeight(const UI_CONTROLS_BACKEND_SELECTOR *self);
static void M_SetPosition(
    UI_CONTROLS_BACKEND_SELECTOR *self, int32_t x, int32_t y);
static void M_Control(UI_CONTROLS_BACKEND_SELECTOR *self);
static void M_Draw(UI_CONTROLS_BACKEND_SELECTOR *self);
static void M_Free(UI_CONTROLS_BACKEND_SELECTOR *self);

static void M_SyncOutlines(UI_CONTROLS_BACKEND_SELECTOR *const self)
{
    UI_Label_RemoveFrame(self->keyboard_label);
    UI_Label_RemoveFrame(self->controller_label);
    switch (self->controller->backend) {
    case INPUT_BACKEND_KEYBOARD:
        UI_Label_AddFrame(self->keyboard_label);
        break;
    case INPUT_BACKEND_CONTROLLER:
        UI_Label_AddFrame(self->controller_label);
        break;
    default:
        break;
    }
}

static int32_t M_GetWidth(const UI_CONTROLS_BACKEND_SELECTOR *const self)
{
    return self->container->get_width(self->container);
}

static int32_t M_GetHeight(const UI_CONTROLS_BACKEND_SELECTOR *const self)
{
    return self->container->get_height(self->container);
}

static void M_SetPosition(
    UI_CONTROLS_BACKEND_SELECTOR *const self, const int32_t x, const int32_t y)
{
    self->container->set_position(self->container, x, y);
}

static void M_Control(UI_CONTROLS_BACKEND_SELECTOR *const self)
{
    M_SyncOutlines(self);
}

static void M_Draw(UI_CONTROLS_BACKEND_SELECTOR *const self)
{
    if (self->container->draw != NULL) {
        self->container->draw(self->container);
    }
}

static void M_Free(UI_CONTROLS_BACKEND_SELECTOR *const self)
{
    self->keyboard_label->free(self->keyboard_label);
    self->controller_label->free(self->controller_label);
    self->container->free(self->container);
    Memory_Free(self);
}

UI_WIDGET *UI_ControlsBackendSelector_Create(
    UI_CONTROLS_CONTROLLER *const controller)
{
    UI_CONTROLS_BACKEND_SELECTOR *const self =
        Memory_Alloc(sizeof(UI_CONTROLS_BACKEND_SELECTOR));
    self->vtable = (UI_WIDGET_VTABLE) {
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->controller = controller;

    self->keyboard_label =
        UI_Label_Create(GS(CONTROL_BACKEND_KEYBOARD), UI_LABEL_AUTO_SIZE, 16);
    self->controller_label =
        UI_Label_Create(GS(CONTROL_BACKEND_CONTROLLER), UI_LABEL_AUTO_SIZE, 16);

    self->container = UI_Stack_Create(
        UI_STACK_LAYOUT_VERTICAL, UI_STACK_AUTO_SIZE, UI_STACK_AUTO_SIZE);

    UI_Stack_AddChild(self->container, self->keyboard_label);
    UI_Stack_AddChild(self->container, self->controller_label);
    UI_Stack_SetHAlign(self->container, UI_STACK_H_ALIGN_CENTER);

    M_SyncOutlines(self);

    return (UI_WIDGET *)self;
}
