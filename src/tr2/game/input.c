#include "game/input.h"

#include "config.h"
#include "game/console/common.h"
#include "game/game_string.h"
#include "game/shell.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/game/input/backends/base.h>
#include <libtrx/game/input/backends/controller.h>
#include <libtrx/game/input/backends/keyboard.h>

static INPUT_STATE M_GetDebounced(INPUT_STATE input);
static void M_UpdateFromBackend(
    INPUT_STATE *s, const INPUT_BACKEND_IMPL *backend, INPUT_LAYOUT layout);

static INPUT_STATE M_GetDebounced(const INPUT_STATE input)
{
    INPUT_STATE result;
    result.any = input.any & ~g_OldInputDB.any;

    g_OldInputDB = input;
    return result;
}

static void M_UpdateFromBackend(
    INPUT_STATE *const s, const INPUT_BACKEND_IMPL *const backend,
    const INPUT_LAYOUT layout)
{
    // clang-format off
    s->forward                     |= backend->is_pressed(layout, INPUT_ROLE_UP);
    s->back                        |= backend->is_pressed(layout, INPUT_ROLE_DOWN);
    s->left                        |= backend->is_pressed(layout, INPUT_ROLE_LEFT);
    s->right                       |= backend->is_pressed(layout, INPUT_ROLE_RIGHT);
    s->step_left                   |= backend->is_pressed(layout, INPUT_ROLE_STEP_L);
    s->step_right                  |= backend->is_pressed(layout, INPUT_ROLE_STEP_R);
    s->slow                        |= backend->is_pressed(layout, INPUT_ROLE_SLOW);
    s->jump                        |= backend->is_pressed(layout, INPUT_ROLE_JUMP);
    s->action                      |= backend->is_pressed(layout, INPUT_ROLE_ACTION);
    s->draw                        |= backend->is_pressed(layout, INPUT_ROLE_DRAW);
    s->look                        |= backend->is_pressed(layout, INPUT_ROLE_LOOK);
    s->roll                        |= backend->is_pressed(layout, INPUT_ROLE_ROLL);

    s->enter_console               |= backend->is_pressed(layout, INPUT_ROLE_ENTER_CONSOLE);
    s->save                        |= backend->is_pressed(layout, INPUT_ROLE_SAVE);
    s->load                        |= backend->is_pressed(layout, INPUT_ROLE_LOAD);

    s->item_cheat                  |= backend->is_pressed(layout, INPUT_ROLE_ITEM_CHEAT);
    s->fly_cheat                   |= backend->is_pressed(layout, INPUT_ROLE_FLY_CHEAT);
    s->level_skip_cheat            |= backend->is_pressed(layout, INPUT_ROLE_LEVEL_SKIP_CHEAT);

    s->equip_pistols               |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_PISTOLS);
    s->equip_shotgun               |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_SHOTGUN);
    s->equip_magnums               |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_MAGNUMS);
    s->equip_uzis                  |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_UZIS);
    s->equip_harpoon               |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_HARPOON);
    s->equip_m16                   |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_M16);
    s->equip_grenade_launcher      |= backend->is_pressed(layout, INPUT_ROLE_EQUIP_GRENADE_LAUNCHER);
    s->use_flare                   |= backend->is_pressed(layout, INPUT_ROLE_USE_FLARE);
    s->use_small_medi              |= backend->is_pressed(layout, INPUT_ROLE_USE_SMALL_MEDI);
    s->use_big_medi                |= backend->is_pressed(layout, INPUT_ROLE_USE_BIG_MEDI);

    s->option                      |= backend->is_pressed(layout, INPUT_ROLE_OPTION);
    s->menu_up                     |= backend->is_pressed(layout, INPUT_ROLE_MENU_UP);
    s->menu_down                   |= backend->is_pressed(layout, INPUT_ROLE_MENU_DOWN);
    s->menu_left                   |= backend->is_pressed(layout, INPUT_ROLE_MENU_LEFT);
    s->menu_right                  |= backend->is_pressed(layout, INPUT_ROLE_MENU_RIGHT);
    s->menu_confirm                |= backend->is_pressed(layout, INPUT_ROLE_MENU_CONFIRM);
    s->menu_back                   |= backend->is_pressed(layout, INPUT_ROLE_MENU_BACK);

    s->screenshot                  |= backend->is_pressed(layout, INPUT_ROLE_SCREENSHOT);
    s->switch_resolution           |= backend->is_pressed(layout, INPUT_ROLE_SWITCH_RESOLUTION);
    s->switch_internal_screen_size |= backend->is_pressed(layout, INPUT_ROLE_SWITCH_INTERNAL_SCREEN_SIZE);
    s->toggle_bilinear_filter      |= backend->is_pressed(layout, INPUT_ROLE_TOGGLE_BILINEAR_FILTER);
    s->toggle_perspective_filter   |= backend->is_pressed(layout, INPUT_ROLE_TOGGLE_PERSPECTIVE_FILTER);
    s->toggle_z_buffer             |= backend->is_pressed(layout, INPUT_ROLE_TOGGLE_Z_BUFFER);
    s->toggle_dither               |= backend->is_pressed(layout, INPUT_ROLE_TOGGLE_DITHER);
    s->toggle_fullscreen           |= backend->is_pressed(layout, INPUT_ROLE_TOGGLE_FULLSCREEN);
    s->toggle_rendering_mode       |= backend->is_pressed(layout, INPUT_ROLE_TOGGLE_RENDERING_MODE);
    // clang-format on

    backend->custom_update(s, layout);
}

void Input_Update(void)
{
    g_Input.any = 0;

    M_UpdateFromBackend(
        &g_Input, &g_Input_Keyboard, g_Config.input.keyboard_layout);
    M_UpdateFromBackend(
        &g_Input, &g_Input_Controller, g_Config.input.controller_layout);

    g_Input.option &= g_Camera.type != CAM_CINEMATIC;
    g_Input.roll |= g_Input.forward && g_Input.back;
    g_Input.menu_back |= g_Input.option;
    if (g_Input.left && g_Input.right) {
        g_Input.left = 0;
        g_Input.right = 0;
    }

    if (!g_Config.gameplay.enable_cheats) {
        g_Input.item_cheat = 0;
        g_Input.fly_cheat = 0;
        g_Input.level_skip_cheat = 0;
    }

    g_InputDB = M_GetDebounced(g_Input);

    if (Input_IsInListenMode()) {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0 };
    }
}

const char *Input_GetRoleName(const INPUT_ROLE role)
{
    // clang-format off
    switch (role) {
    case INPUT_ROLE_UP:               return GS(KEYMAP_RUN);
    case INPUT_ROLE_DOWN:             return GS(KEYMAP_BACK);
    case INPUT_ROLE_LEFT:             return GS(KEYMAP_LEFT);
    case INPUT_ROLE_RIGHT:            return GS(KEYMAP_RIGHT);
    case INPUT_ROLE_STEP_L:           return GS(KEYMAP_STEP_LEFT);
    case INPUT_ROLE_STEP_R:           return GS(KEYMAP_STEP_RIGHT);
    case INPUT_ROLE_SLOW:             return GS(KEYMAP_WALK);
    case INPUT_ROLE_JUMP:             return GS(KEYMAP_JUMP);
    case INPUT_ROLE_ACTION:           return GS(KEYMAP_ACTION);
    case INPUT_ROLE_DRAW:             return GS(KEYMAP_DRAW_WEAPON);
    case INPUT_ROLE_USE_FLARE:        return GS(KEYMAP_USE_FLARE);
    case INPUT_ROLE_LOOK:             return GS(KEYMAP_LOOK);
    case INPUT_ROLE_ROLL:             return GS(KEYMAP_ROLL);
    case INPUT_ROLE_OPTION:           return GS(KEYMAP_INVENTORY);
    case INPUT_ROLE_FLY_CHEAT:        return GS(KEYMAP_FLY_CHEAT);
    case INPUT_ROLE_ITEM_CHEAT:       return GS(KEYMAP_ITEM_CHEAT);
    case INPUT_ROLE_LEVEL_SKIP_CHEAT: return GS(KEYMAP_LEVEL_SKIP_CHEAT);
    case INPUT_ROLE_ENTER_CONSOLE:    return GS(KEYMAP_ENTER_CONSOLE);
    default:                          return "";
    }
    // clang-format on
}
