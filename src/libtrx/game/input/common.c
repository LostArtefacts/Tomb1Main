#include "game/input/common.h"

#include "enum_map.h"
#include "game/clock.h"
#include "game/game_string.h"
#include "game/input/backends/controller.h"
#include "game/input/backends/keyboard.h"

#include <stdint.h>

#define DELAY_TIME 0.4
#define HOLD_TIME 0.1

INPUT_STATE g_Input = {};
INPUT_STATE g_InputDB = {};
INPUT_STATE g_OldInputDB = {};

static CLOCK_TIMER m_HoldBackTimer = { .type = CLOCK_TIMER_REAL };
static CLOCK_TIMER m_HoldForwardTimer = { .type = CLOCK_TIMER_REAL };
static bool m_ListenMode = false;

static bool m_IsRoleHardcoded[INPUT_ROLE_NUMBER_OF] = {
    0,
#if TR_VERSION == 1
    [INPUT_ROLE_UNBIND_KEY] = 1,
    [INPUT_ROLE_RESET_BINDINGS] = 1,
    [INPUT_ROLE_PERSPECTIVE] = 1,
#endif
    [INPUT_ROLE_MENU_CONFIRM] = 1,
    [INPUT_ROLE_MENU_BACK] = 1,
    [INPUT_ROLE_MENU_LEFT] = 1,
    [INPUT_ROLE_MENU_RIGHT] = 1,
    [INPUT_ROLE_MENU_UP] = 1,
    [INPUT_ROLE_MENU_DOWN] = 1,
};

static const GAME_STRING_ID m_LayoutMap[INPUT_LAYOUT_NUMBER_OF] = {
    [INPUT_LAYOUT_DEFAULT] = GS_ID(CONTROL_DEFAULT_KEYS),
    [INPUT_LAYOUT_CUSTOM_1] = GS_ID(CONTROL_CUSTOM_1),
    [INPUT_LAYOUT_CUSTOM_2] = GS_ID(CONTROL_CUSTOM_2),
    [INPUT_LAYOUT_CUSTOM_3] = GS_ID(CONTROL_CUSTOM_3),
};

static INPUT_BACKEND_IMPL *M_GetBackend(INPUT_BACKEND backend);

static INPUT_BACKEND_IMPL *M_GetBackend(const INPUT_BACKEND backend)
{
    switch (backend) {
    case INPUT_BACKEND_KEYBOARD:
        return &g_Input_Keyboard;
    case INPUT_BACKEND_CONTROLLER:
        return &g_Input_Controller;
    default:
        return NULL;
    }
}

void Input_Init(void)
{
    if (g_Input_Keyboard.init != NULL) {
        g_Input_Keyboard.init();
    }
    if (g_Input_Controller.init != NULL) {
        g_Input_Controller.init();
    }
}

void Input_Shutdown(void)
{
    if (g_Input_Keyboard.shutdown != NULL) {
        g_Input_Keyboard.shutdown();
    }
    if (g_Input_Controller.shutdown != NULL) {
        g_Input_Controller.shutdown();
    }
}

void Input_InitController(void)
{
    if (g_Input_Controller.init != NULL) {
        g_Input_Controller.init();
    }
}

void Input_ShutdownController(void)
{
    if (g_Input_Controller.shutdown != NULL) {
        g_Input_Controller.shutdown();
    }
}

bool Input_IsRoleRebindable(const INPUT_ROLE role)
{
    return !m_IsRoleHardcoded[role];
}

bool Input_IsPressed(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    const INPUT_ROLE role)
{
    return M_GetBackend(backend)->is_pressed(layout, role);
}

bool Input_IsKeyConflicted(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    const INPUT_ROLE role)
{
    return M_GetBackend(backend)->is_role_conflicted(layout, role);
}

bool Input_ReadAndAssignRole(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    const INPUT_ROLE role)
{
    // Check for canceling from other devices
    for (INPUT_BACKEND other_backend = 0;
         other_backend < INPUT_BACKEND_NUMBER_OF; other_backend++) {
        if (other_backend == backend) {
            continue;
        }
        if (Input_IsPressed(other_backend, layout, INPUT_ROLE_MENU_BACK)
            || Input_IsPressed(other_backend, layout, INPUT_ROLE_OPTION)) {
            return true;
        }
    }

    return M_GetBackend(backend)->read_and_assign(layout, role);
}

void Input_UnassignRole(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    const INPUT_ROLE role)
{
    M_GetBackend(backend)->unassign_role(layout, role);
}

const char *Input_GetKeyName(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    const INPUT_ROLE role)
{
    return M_GetBackend(backend)->get_name(layout, role);
}

void Input_ResetLayout(const INPUT_BACKEND backend, const INPUT_LAYOUT layout)
{
    M_GetBackend(backend)->reset_layout(layout);
}

void Input_EnterListenMode(void)
{
    m_ListenMode = true;
}

void Input_ExitListenMode(void)
{
    m_ListenMode = false;
    Input_Update();
    g_OldInputDB.any = g_Input.any;
    g_InputDB.any = g_Input.any;
}

bool Input_IsInListenMode(void)
{
    return m_ListenMode;
}

bool Input_AssignFromJSONObject(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    JSON_OBJECT *const bind_obj)
{
    INPUT_ROLE role = (INPUT_ROLE)-1;

#if TR_VERSION == 1
    // TR1X <=4.5, TR2X <=0.5
    const int32_t role_idx = JSON_ObjectGetInt(bind_obj, "role", -1);
    // clang-format off
    switch (role_idx) {
    case 0: role = INPUT_ROLE_UP; break;
    case 1: role = INPUT_ROLE_DOWN; break;
    case 2: role = INPUT_ROLE_LEFT; break;
    case 3: role = INPUT_ROLE_RIGHT; break;
    case 4: role = INPUT_ROLE_STEP_L; break;
    case 5: role = INPUT_ROLE_STEP_R; break;
    case 6: role = INPUT_ROLE_SLOW; break;
    case 7: role = INPUT_ROLE_JUMP; break;
    case 8: role = INPUT_ROLE_ACTION; break;
    case 9: role = INPUT_ROLE_DRAW; break;
    case 10: role = INPUT_ROLE_LOOK; break;
    case 11: role = INPUT_ROLE_ROLL; break;
    case 12: role = INPUT_ROLE_OPTION; break;
    case 13: role = INPUT_ROLE_FLY_CHEAT; break;
    case 14: role = INPUT_ROLE_ITEM_CHEAT; break;
    case 15: role = INPUT_ROLE_LEVEL_SKIP_CHEAT; break;
    case 16: role = INPUT_ROLE_TURBO_CHEAT; break;
    case 17: role = INPUT_ROLE_PAUSE; break;
    case 18: role = INPUT_ROLE_CAMERA_FORWARD; break;
    case 19: role = INPUT_ROLE_CAMERA_BACK; break;
    case 20: role = INPUT_ROLE_CAMERA_LEFT; break;
    case 21: role = INPUT_ROLE_CAMERA_RIGHT; break;
    case 22: role = INPUT_ROLE_CAMERA_RESET; break;
    case 23: role = INPUT_ROLE_EQUIP_PISTOLS; break;
    case 24: role = INPUT_ROLE_EQUIP_SHOTGUN; break;
    case 25: role = INPUT_ROLE_EQUIP_MAGNUMS; break;
    case 26: role = INPUT_ROLE_EQUIP_UZIS; break;
    case 27: role = INPUT_ROLE_USE_SMALL_MEDI; break;
    case 28: role = INPUT_ROLE_USE_BIG_MEDI; break;
    case 29: role = INPUT_ROLE_SAVE; break;
    case 30: role = INPUT_ROLE_LOAD; break;
    case 31: role = INPUT_ROLE_FPS; break;
    case 32: role = INPUT_ROLE_BILINEAR; break;
    case 33: role = INPUT_ROLE_ENTER_CONSOLE; break;
    case 34: role = INPUT_ROLE_CHANGE_TARGET; break;
    case 35: role = INPUT_ROLE_TOGGLE_UI; break;
    case 36: role = INPUT_ROLE_CAMERA_UP; break;
    case 37: role = INPUT_ROLE_CAMERA_DOWN; break;
    case 38: role = INPUT_ROLE_TOGGLE_PHOTO_MODE; break;
    case 39: role = INPUT_ROLE_UNBIND_KEY; break;
    case 40: role = INPUT_ROLE_RESET_BINDINGS; break;
    case 42: role = INPUT_ROLE_PERSPECTIVE; break;
    case 43: role = INPUT_ROLE_MENU_CONFIRM; break;
    case 44: role = INPUT_ROLE_MENU_BACK; break;
    case 45: role = INPUT_ROLE_MENU_LEFT; break;
    case 46: role = INPUT_ROLE_MENU_UP; break;
    case 47: role = INPUT_ROLE_MENU_DOWN; break;
    case 48: role = INPUT_ROLE_MENU_RIGHT; break;
    case 49: role = INPUT_ROLE_SCREENSHOT; break;
    case 50: role = INPUT_ROLE_TOGGLE_FULLSCREEN; break;
    }
    // clang-format on
#endif

    // TR1X >= 4.6, TR2X >= 0.6
    if (role == (INPUT_ROLE)-1) {
        role = ENUM_MAP_GET(
            INPUT_ROLE, JSON_ObjectGetString(bind_obj, "role", ""),
            (int32_t)(INPUT_ROLE)-1);
    }

    if (role == (INPUT_ROLE)-1) {
        return false;
    }

    return M_GetBackend(backend)->assign_from_json_object(
        layout, role, bind_obj);
}

bool Input_AssignToJSONObject(
    const INPUT_BACKEND backend, const INPUT_LAYOUT layout,
    JSON_OBJECT *const bind_obj, const INPUT_ROLE role)
{
    JSON_ObjectAppendString(
        bind_obj, "role", ENUM_MAP_TO_STRING(INPUT_ROLE, role));
    return M_GetBackend(backend)->assign_to_json_object(layout, role, bind_obj);
}

const char *Input_GetLayoutName(const INPUT_LAYOUT layout)
{
    return GameString_Get(m_LayoutMap[layout]);
}

INPUT_STATE Input_GetDebounced(const INPUT_STATE input)
{
    INPUT_STATE result;
    result.any = input.any & ~g_OldInputDB.any;

    // Allow holding down key to move faster
    if (input.forward || !input.back) {
        m_HoldBackTimer.ref = 0.0;
    } else if (input.back && m_HoldBackTimer.ref == 0.0) {
        ClockTimer_Sync(&m_HoldBackTimer);
    } else if (
        input.back
        && ClockTimer_CheckElapsedAndTake(
            &m_HoldBackTimer, DELAY_TIME + HOLD_TIME)) {
        result.back = 1;
        result.menu_down = 1;
        ClockTimer_Sync(&m_HoldBackTimer);
        m_HoldBackTimer.ref -= DELAY_TIME;
    }

    if (!input.forward || input.back) {
        m_HoldForwardTimer.ref = 0.0;
    } else if (input.forward && m_HoldForwardTimer.ref == 0.0) {
        ClockTimer_Sync(&m_HoldForwardTimer);
    } else if (
        input.forward
        && ClockTimer_CheckElapsed(
            &m_HoldForwardTimer, DELAY_TIME + HOLD_TIME)) {
        result.forward = 1;
        result.menu_up = 1;
        ClockTimer_Sync(&m_HoldForwardTimer);
        m_HoldForwardTimer.ref -= DELAY_TIME;
    }

    g_OldInputDB = input;
    return result;
}
