#include "game/ui/widgets/controls_input_selector.h"

#include <libtrx/game/ui/widgets/label.h>
#include <libtrx/game/ui/widgets/stack.h>
#include <libtrx/memory.h>

#include <stdio.h>
#include <string.h>

typedef struct {
    UI_WIDGET_VTABLE vtable;
    INPUT_ROLE input_role;
    UI_WIDGET *label;
    UI_WIDGET *choice;
    UI_WIDGET *container;
    UI_CONTROLS_CONTROLLER *controller;
} UI_CONTROLS_INPUT_SELECTOR;

static void M_UpdateText(UI_CONTROLS_INPUT_SELECTOR *self);
static int32_t M_GetWidth(const UI_CONTROLS_INPUT_SELECTOR *self);
static int32_t M_GetHeight(const UI_CONTROLS_INPUT_SELECTOR *self);
static void M_SetPosition(
    UI_CONTROLS_INPUT_SELECTOR *self, int32_t x, int32_t y);
static void M_Control(UI_CONTROLS_INPUT_SELECTOR *self);
static void M_Draw(UI_CONTROLS_INPUT_SELECTOR *self);
static void M_Free(UI_CONTROLS_INPUT_SELECTOR *self);

static void M_UpdateText(UI_CONTROLS_INPUT_SELECTOR *const self)
{
    const char *const key_name = Input_GetKeyName(
        self->controller->backend, self->controller->active_layout,
        self->input_role);
    UI_Label_ChangeText(self->choice, key_name);
    const char *const role_name = Input_GetRoleName(self->input_role);
    char role_name_padded[strlen(role_name) + 2];
    sprintf(role_name_padded, "%s  ", role_name);
    UI_Label_ChangeText(self->label, role_name_padded);
}

static int32_t M_GetWidth(const UI_CONTROLS_INPUT_SELECTOR *const self)
{
    return self->container->get_width(self->container);
}

static int32_t M_GetHeight(const UI_CONTROLS_INPUT_SELECTOR *const self)
{
    return self->container->get_height(self->container);
}

static void M_SetPosition(
    UI_CONTROLS_INPUT_SELECTOR *const self, const int32_t x, const int32_t y)
{
    self->container->set_position(self->container, x, y);
}

static void M_Control(UI_CONTROLS_INPUT_SELECTOR *const self)
{
    if (self->label->control != nullptr) {
        self->label->control(self->label);
    }
    if (self->choice->control != nullptr) {
        self->choice->control(self->choice);
    }

    // Sync outlines
    UI_Label_RemoveFrame(self->label);
    UI_Label_RemoveFrame(self->choice);
    if (self->controller->active_role == self->input_role) {
        if (self->controller->state == UI_CONTROLS_STATE_NAVIGATE_INPUTS
            || self->controller->state
                == UI_CONTROLS_STATE_NAVIGATE_INPUTS_DEBOUNCE) {
            UI_Label_AddFrame(self->label);
        } else if (self->controller->state == UI_CONTROLS_STATE_LISTEN) {
            UI_Label_AddFrame(self->choice);
        }
    }

    M_UpdateText(self);

    // Flash conflicts
    UI_Label_Flash(self->choice, false, 0);
    if (Input_IsKeyConflicted(
            self->controller->backend, self->controller->active_layout,
            self->input_role)) {
        UI_Label_Flash(self->choice, true, 20);
    }
}

static void M_Draw(UI_CONTROLS_INPUT_SELECTOR *const self)
{
    if (self->label->draw != nullptr) {
        self->label->draw(self->label);
    }
    if (self->choice->draw != nullptr) {
        self->choice->draw(self->choice);
    }
}

static void M_Free(UI_CONTROLS_INPUT_SELECTOR *const self)
{
    self->label->free(self->label);
    self->choice->free(self->choice);
    self->container->free(self->container);
    Memory_Free(self);
}

UI_WIDGET *UI_ControlsInputSelector_Create(
    const INPUT_ROLE input_role, UI_CONTROLS_CONTROLLER *const controller)
{
    UI_CONTROLS_INPUT_SELECTOR *const self =
        Memory_Alloc(sizeof(UI_CONTROLS_INPUT_SELECTOR));
    self->vtable = (UI_WIDGET_VTABLE) {
        .get_width = (UI_WIDGET_GET_WIDTH)M_GetWidth,
        .get_height = (UI_WIDGET_GET_HEIGHT)M_GetHeight,
        .set_position = (UI_WIDGET_SET_POSITION)M_SetPosition,
        .control = (UI_WIDGET_CONTROL)M_Control,
        .draw = (UI_WIDGET_DRAW)M_Draw,
        .free = (UI_WIDGET_FREE)M_Free,
    };

    self->controller = controller;
    self->input_role = input_role;

    self->label = UI_Label_Create("", UI_LABEL_AUTO_SIZE, 15);
    self->choice = UI_Label_Create("", 70, 15);
    self->container = UI_Stack_Create(
        UI_STACK_LAYOUT_HORIZONTAL, UI_STACK_AUTO_SIZE, UI_STACK_AUTO_SIZE);
    UI_Stack_AddChild(self->container, self->choice);
    UI_Stack_AddChild(self->container, self->label);

    // update the text on init
    M_UpdateText(self);

    return (UI_WIDGET *)self;
}
