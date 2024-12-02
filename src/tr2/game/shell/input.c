#include "game/shell/input.h"

#include "config.h"
#include "decomp/decomp.h"
#include "game/clock.h"
#include "game/console/common.h"
#include "game/gameflow/gameflow_new.h"
#include "game/input.h"
#include "game/inventory/backpack.h"
#include "game/lara/control.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/screenshot.h>
#include <libtrx/utils.h>

static void M_ToggleBilinearFiltering(void);
static void M_TogglePerspectiveCorrection(void);
static void M_ToggleZBuffer(void);
static void M_ToggleTripleBuffering(void);
static void M_ToggleDither(void);
static void M_ToggleFullscreen(void);
static void M_ToggleRenderingMode(void);
static void M_DecreaseResolutionOrBPP(void);
static void M_IncreaseResolutionOrBPP(void);
static void M_DecreaseInternalScreenSize(void);
static void M_IncreaseInternalScreenSize(void);

static void M_ToggleBilinearFiltering(void)
{
    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.bilinear_filtering = !new_settings.bilinear_filtering;
    GameApplySettings(&new_settings);
}

static void M_TogglePerspectiveCorrection(void)
{
    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.perspective_correct = !new_settings.perspective_correct;
    g_PerspectiveDistance = g_SavedAppSettings.perspective_correct
        ? SW_DETAIL_HIGH
        : SW_DETAIL_MEDIUM;
    GameApplySettings(&new_settings);
}

static void M_ToggleZBuffer(void)
{
    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.zbuffer = !new_settings.zbuffer;
    GameApplySettings(&new_settings);
}

static void M_ToggleTripleBuffering(void)
{
    if (g_SavedAppSettings.fullscreen) {
        return;
    }

    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.triple_buffering = !new_settings.triple_buffering;
    GameApplySettings(&new_settings);
}

static void M_ToggleDither(void)
{
    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.dither = !new_settings.dither;
    GameApplySettings(&new_settings);
}

static void M_ToggleFullscreen(void)
{
    if (g_IsVidModeLock) {
        return;
    }

    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.fullscreen = !new_settings.fullscreen;
    if (g_SavedAppSettings.fullscreen) {
        const int32_t win_width = MAX(g_PhdWinWidth, 320);
        const int32_t win_height = MAX(g_PhdWinHeight, 240);
        new_settings.window_height = win_height;
        new_settings.window_width = CalculateWindowWidth(win_width, win_height);
        new_settings.triple_buffering = 0;
        GameApplySettings(&new_settings);
        setup_screen_size();
    } else {
        const DISPLAY_MODE_LIST *const mode_list =
            new_settings.render_mode == RM_HARDWARE
            ? &g_CurrentDisplayAdapter.hw_disp_mode_list
            : &g_CurrentDisplayAdapter.sw_disp_mode_list;

        if (mode_list->count > 0) {
            const DISPLAY_MODE target_mode = {
                .width = g_GameVid_Width,
                .height = g_GameVid_Height,
                .bpp = g_GameVid_BPP,
                .vga = VGA_NO_VGA,
            };

            const DISPLAY_MODE_NODE *mode = NULL;
            for (mode = mode_list->head; mode != NULL; mode = mode->next) {
                if (!CompareVideoModes(&mode->body, &target_mode)) {
                    break;
                }
            }

            if (mode == NULL) {
                mode = mode_list->tail;
            }

            new_settings.video_mode = mode;
            GameApplySettings(&new_settings);
        }
    }
}

static void M_ToggleRenderingMode(void)
{
    if (g_IsVidModeLock || g_Inv_IsActive) {
        return;
    }

    APP_SETTINGS new_settings = g_SavedAppSettings;
    new_settings.render_mode =
        new_settings.render_mode == RM_HARDWARE ? RM_SOFTWARE : RM_HARDWARE;

    const DISPLAY_MODE_LIST *const mode_list =
        new_settings.render_mode == RM_HARDWARE
        ? &g_CurrentDisplayAdapter.hw_disp_mode_list
        : &g_CurrentDisplayAdapter.sw_disp_mode_list;

    if (mode_list->count == 0) {
        return;
    }

    const DISPLAY_MODE target_mode = {
        .width = g_GameVid_Width,
        .height = g_GameVid_Height,
        .bpp = new_settings.render_mode == RM_HARDWARE ? 16 : 8,
        .vga = VGA_NO_VGA,
    };

    const DISPLAY_MODE_NODE *mode = NULL;
    for (mode = mode_list->head; mode != NULL; mode = mode->next) {
        if (!CompareVideoModes(&mode->body, &target_mode)) {
            break;
        }
    }

    if (mode == NULL) {
        mode = mode_list->tail;
    }

    new_settings.video_mode = mode;
    new_settings.fullscreen = 1;
    GameApplySettings(&new_settings);
}

static void M_DecreaseResolutionOrBPP(void)
{
    if (g_IsVidSizeLock || g_Camera.type == CAM_CINEMATIC
        || g_GameFlow.screen_sizing_disabled
        || !g_SavedAppSettings.fullscreen) {
        return;
    }

    APP_SETTINGS new_settings = g_SavedAppSettings;
    const DISPLAY_MODE_NODE *const current_mode = new_settings.video_mode;
    const DISPLAY_MODE_NODE *mode = current_mode;
    if (mode != NULL) {
        mode = mode->previous;
    }

    if (new_settings.render_mode == RM_HARDWARE) {
        for (; mode != NULL; mode = mode->previous) {
            if (g_Input.slow) {
                if (mode->body.width == current_mode->body.width
                    && mode->body.height == current_mode->body.height
                    && mode->body.vga == current_mode->body.vga
                    && mode->body.bpp < current_mode->body.bpp) {
                    break;
                }
            } else if (
                mode->body.vga == current_mode->body.vga
                && mode->body.bpp == current_mode->body.bpp) {
                break;
            }
        }
    }

    if (mode != NULL) {
        new_settings.video_mode = mode;
        GameApplySettings(&new_settings);
    }
}

static void M_IncreaseResolutionOrBPP(void)
{
    if (g_IsVidSizeLock || g_Camera.type == CAM_CINEMATIC
        || g_GameFlow.screen_sizing_disabled
        || !g_SavedAppSettings.fullscreen) {
        return;
    }

    APP_SETTINGS new_settings = g_SavedAppSettings;
    const DISPLAY_MODE_NODE *const current_mode = new_settings.video_mode;
    const DISPLAY_MODE_NODE *mode = current_mode;
    if (mode != NULL) {
        mode = mode->next;
    }

    if (new_settings.render_mode == RM_HARDWARE) {
        for (; mode != NULL; mode = mode->next) {
            if (g_Input.slow) {
                if (mode->body.width == current_mode->body.width
                    && mode->body.height == current_mode->body.height
                    && mode->body.vga == current_mode->body.vga
                    && mode->body.bpp > current_mode->body.bpp) {
                    break;
                }
            } else if (
                mode->body.vga == current_mode->body.vga
                && mode->body.bpp == current_mode->body.bpp) {
                break;
            }
        }
    }
    if (mode != NULL) {
        new_settings.video_mode = mode;
        GameApplySettings(&new_settings);
    }
}

static void M_DecreaseInternalScreenSize(void)
{
    if (g_IsVidSizeLock || g_Camera.type == CAM_CINEMATIC
        || g_GameFlow.screen_sizing_disabled
        || !g_SavedAppSettings.fullscreen) {
        return;
    }

    DecreaseScreenSize();
}

static void M_IncreaseInternalScreenSize(void)
{
    if (g_IsVidSizeLock || g_Camera.type == CAM_CINEMATIC
        || g_GameFlow.screen_sizing_disabled
        || !g_SavedAppSettings.fullscreen) {
        return;
    }

    IncreaseScreenSize();
}

void Shell_ProcessInput(void)
{
    if (g_InputDB.screenshot) {
        Screenshot_Make(g_Config.rendering.screenshot_format);
    }

    if (g_InputDB.switch_resolution) {
        if (g_Input.slow) {
            M_DecreaseResolutionOrBPP();
        } else {
            M_IncreaseResolutionOrBPP();
        }
    }

    if (g_InputDB.switch_internal_screen_size) {
        if (g_Input.slow) {
            M_DecreaseInternalScreenSize();
        } else {
            M_IncreaseInternalScreenSize();
        }
    }

    if (g_InputDB.toggle_bilinear_filter) {
        M_ToggleBilinearFiltering();
    }

    if (g_InputDB.toggle_perspective_filter) {
        M_TogglePerspectiveCorrection();
    }

    if (g_InputDB.toggle_z_buffer) {
        M_ToggleZBuffer();
    }

    if (g_InputDB.toggle_dither) {
        M_ToggleDither();
    }

    if (g_InputDB.toggle_fullscreen) {
        M_ToggleFullscreen();
    }

    if (g_InputDB.toggle_rendering_mode) {
        M_ToggleRenderingMode();
    }

    if (g_InputDB.turbo_cheat) {
        Clock_CycleTurboSpeed(!g_Input.slow);
    }
}
