#include "game/ui/controllers/controls.h"

#include "game/input.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/game/ui/events.h>
#include <libtrx/utils.h>

static const INPUT_ROLE m_LeftRoles[] = {
    // clang-format off
    INPUT_ROLE_UP,
    INPUT_ROLE_DOWN,
    INPUT_ROLE_LEFT,
    INPUT_ROLE_RIGHT,
    INPUT_ROLE_STEP_L,
    INPUT_ROLE_STEP_R,
    INPUT_ROLE_SLOW,
    INPUT_ROLE_ENTER_CONSOLE,
    INPUT_ROLE_PAUSE,
    (INPUT_ROLE)-1,
    // clang-format on
};

static const INPUT_ROLE m_RightRoles_CheatsOff[] = {
    // clang-format off
    INPUT_ROLE_JUMP,
    INPUT_ROLE_ACTION,
    INPUT_ROLE_DRAW,
    INPUT_ROLE_USE_FLARE,
    INPUT_ROLE_LOOK,
    INPUT_ROLE_ROLL,
    INPUT_ROLE_OPTION,
    (INPUT_ROLE)-1,
    // clang-format on
};

static const INPUT_ROLE m_RightRoles_CheatsOn[] = {
    // clang-format off
    INPUT_ROLE_JUMP,
    INPUT_ROLE_ACTION,
    INPUT_ROLE_DRAW,
    INPUT_ROLE_USE_FLARE,
    INPUT_ROLE_LOOK,
    INPUT_ROLE_ROLL,
    INPUT_ROLE_OPTION,
    INPUT_ROLE_FLY_CHEAT,
    INPUT_ROLE_ITEM_CHEAT,
    INPUT_ROLE_LEVEL_SKIP_CHEAT,
    INPUT_ROLE_TURBO_CHEAT,
    (INPUT_ROLE)-1,
    // clang-format on
};

static INPUT_ROLE M_GetInputRole(int32_t col, int32_t row);
static void M_CycleLayout(UI_CONTROLS_CONTROLLER *controller, int32_t dir);
static bool M_NavigateLayout(UI_CONTROLS_CONTROLLER *controller);
static bool M_NavigateInputs(UI_CONTROLS_CONTROLLER *controller);
static bool M_NavigateInputsDebounce(UI_CONTROLS_CONTROLLER *controller);
static bool M_Listen(UI_CONTROLS_CONTROLLER *controller);
static bool M_ListenDebounce(UI_CONTROLS_CONTROLLER *controller);

static INPUT_ROLE M_GetInputRole(const int32_t col, const int32_t row)
{
    if (col == 0) {
        return m_LeftRoles[row];
    } else if (g_Config.gameplay.enable_cheats) {
        return m_RightRoles_CheatsOn[row];
    } else {
        return m_RightRoles_CheatsOff[row];
    }
}

static void M_CycleLayout(
    UI_CONTROLS_CONTROLLER *const controller, const int32_t dir)
{
    controller->active_layout += dir;
    controller->active_layout += INPUT_LAYOUT_NUMBER_OF;
    controller->active_layout %= INPUT_LAYOUT_NUMBER_OF;

    const EVENT event = {
        .name = "layout_change",
        .sender = NULL,
        .data = NULL,
    };
    EventManager_Fire(controller->events, &event);
}

static bool M_NavigateBackend(UI_CONTROLS_CONTROLLER *const controller)
{
    if (g_InputDB.menu_down && controller->backend == INPUT_BACKEND_KEYBOARD) {
        controller->backend = INPUT_BACKEND_CONTROLLER;
        return true;
    }

    if (g_InputDB.menu_up && controller->backend == INPUT_BACKEND_CONTROLLER) {
        controller->backend = INPUT_BACKEND_KEYBOARD;
        return true;
    }

    if (g_InputDB.menu_back) {
        controller->state = UI_CONTROLS_STATE_EXIT;
        return true;
    }

    if (g_InputDB.menu_confirm) {
        controller->state = UI_CONTROLS_STATE_NAVIGATE_LAYOUT;
        return true;
    }

    return false;
}

static bool M_NavigateLayout(UI_CONTROLS_CONTROLLER *const controller)
{
    if (g_InputDB.menu_confirm) {
        controller->state = UI_CONTROLS_STATE_EXIT;
    } else if (g_InputDB.menu_back) {
        controller->state = UI_CONTROLS_STATE_NAVIGATE_BACKEND;
    } else if (g_InputDB.left) {
        M_CycleLayout(controller, -1);
    } else if (g_InputDB.right) {
        M_CycleLayout(controller, 1);
    } else if (g_InputDB.back && controller->active_layout != 0) {
        controller->state = UI_CONTROLS_STATE_NAVIGATE_INPUTS;
        controller->active_col = 0;
        controller->active_row = 0;
    } else if (g_InputDB.forward && controller->active_layout != 0) {
        controller->state = UI_CONTROLS_STATE_NAVIGATE_INPUTS;
        controller->active_col = 1;
        controller->active_row = UI_ControlsController_GetInputRoleCount(1) - 1;
    } else {
        return false;
    }
    controller->active_role =
        M_GetInputRole(controller->active_col, controller->active_row);
    return true;
}

static bool M_NavigateInputs(UI_CONTROLS_CONTROLLER *const controller)
{
    if (g_InputDB.menu_back) {
        controller->state = UI_CONTROLS_STATE_NAVIGATE_BACKEND;
    } else if (g_InputDB.left || g_InputDB.right) {
        controller->active_col ^= 1;
        CLAMP(
            controller->active_row, 0,
            UI_ControlsController_GetInputRoleCount(controller->active_col)
                - 1);
    } else if (g_InputDB.forward) {
        controller->active_row--;
        if (controller->active_row < 0) {
            if (controller->active_col == 0) {
                controller->state = UI_CONTROLS_STATE_NAVIGATE_LAYOUT;
            } else {
                controller->active_col = 0;
                controller->active_row =
                    UI_ControlsController_GetInputRoleCount(0) - 1;
            }
        }
    } else if (g_InputDB.back) {
        controller->active_row++;
        if (controller->active_row >= UI_ControlsController_GetInputRoleCount(
                controller->active_col)) {
            if (controller->active_col == 0) {
                controller->active_col = 1;
                controller->active_row = 0;
            } else {
                controller->state = UI_CONTROLS_STATE_NAVIGATE_LAYOUT;
            }
        }
    } else if (g_InputDB.menu_confirm) {
        controller->state = UI_CONTROLS_STATE_NAVIGATE_INPUTS_DEBOUNCE;
    } else {
        return false;
    }
    controller->active_role =
        M_GetInputRole(controller->active_col, controller->active_row);
    return true;
}

static bool M_NavigateInputsDebounce(UI_CONTROLS_CONTROLLER *const controller)
{
    Shell_ProcessEvents();
    Input_Update();
    if (g_Input.any) {
        return false;
    }
    Input_EnterListenMode();
    controller->state = UI_CONTROLS_STATE_LISTEN;
    return true;
}

static bool M_Listen(UI_CONTROLS_CONTROLLER *const controller)
{
    if (!Input_ReadAndAssignRole(
            controller->backend, controller->active_layout,
            controller->active_role)) {
        return false;
    }

    Input_ExitListenMode();

    const EVENT event = {
        .name = "key_change",
        .sender = NULL,
        .data = NULL,
    };
    EventManager_Fire(controller->events, &event);

    controller->state = UI_CONTROLS_STATE_LISTEN_DEBOUNCE;
    return true;
}

static bool M_ListenDebounce(UI_CONTROLS_CONTROLLER *const controller)
{
    if (g_Input.any) {
        return false;
    }

    controller->state = UI_CONTROLS_STATE_NAVIGATE_INPUTS;
    return true;
}

void UI_ControlsController_Init(UI_CONTROLS_CONTROLLER *const controller)
{
    ASSERT(controller->events == NULL);
    controller->backend = INPUT_BACKEND_KEYBOARD;
    controller->state = UI_CONTROLS_STATE_NAVIGATE_BACKEND;

    controller->events = EventManager_Create();
}

void UI_ControlsController_Shutdown(UI_CONTROLS_CONTROLLER *const controller)
{
    EventManager_Free(controller->events);
    controller->events = NULL;
}

bool UI_ControlsController_Control(UI_CONTROLS_CONTROLLER *const controller)
{
    switch (controller->state) {
    case UI_CONTROLS_STATE_NAVIGATE_BACKEND:
        return M_NavigateBackend(controller);
    case UI_CONTROLS_STATE_NAVIGATE_LAYOUT:
        return M_NavigateLayout(controller);
    case UI_CONTROLS_STATE_NAVIGATE_INPUTS:
        return M_NavigateInputs(controller);
    case UI_CONTROLS_STATE_NAVIGATE_INPUTS_DEBOUNCE:
        return M_NavigateInputsDebounce(controller);
    case UI_CONTROLS_STATE_LISTEN:
        return M_Listen(controller);
    case UI_CONTROLS_STATE_LISTEN_DEBOUNCE:
        return M_ListenDebounce(controller);
    default:
        return false;
    }
    return false;
}

INPUT_ROLE UI_ControlsController_GetInputRole(
    const int32_t col, const int32_t row)
{
    return M_GetInputRole(col, row);
}

int32_t UI_ControlsController_GetInputRoleCount(const int32_t col)
{
    int32_t row = 0;
    while (M_GetInputRole(col, row) != (INPUT_ROLE)-1) {
        row++;
    }
    return row;
}
