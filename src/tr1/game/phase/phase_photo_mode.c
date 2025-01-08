#include "game/phase/phase_photo_mode.h"

#include "game/camera.h"
#include "game/game.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/music.h"
#include "game/overlay.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/ui/widgets/photo_mode.h"

#include <libtrx/config.h>
#include <libtrx/game/console/common.h>
#include <libtrx/game/game_string.h>
#include <libtrx/game/ui/common.h>
#include <libtrx/memory.h>

#include <stdio.h>

typedef enum {
    PS_NONE,
    PS_ACTIVE,
    PS_COOLDOWN,
} PHOTO_STATUS;

typedef struct {
    PHOTO_STATUS status;
    UI_WIDGET *ui;
    bool show_fps_counter;
} M_PRIV;

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t num_frames);
static void M_Draw(PHASE *phase);

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    p->status = PS_NONE;
    p->show_fps_counter = g_Config.rendering.enable_fps_counter;
    g_Config.rendering.enable_fps_counter = false;

    g_OldInputDB = g_Input;
    Camera_EnterPhotoMode();
    Overlay_HideGameInfo();
    Music_Pause();
    Sound_PauseAll();

    p->ui = UI_PhotoMode_Create();
    if (!g_Config.ui.enable_photo_mode_ui) {
        Console_Log(
            GS(OSD_PHOTO_MODE_LAUNCHED),
            Input_GetKeyName(
                INPUT_BACKEND_KEYBOARD, g_Config.input.keyboard_layout,
                INPUT_ROLE_TOGGLE_UI));
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Camera_ExitPhotoMode();

    p->ui->free(p->ui);
    p->ui = NULL;

    g_Config.rendering.enable_fps_counter = p->show_fps_counter;
    g_Input = g_OldInputDB;
    Music_Unpause();
    Sound_UnpauseAll();
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t num_frames)
{
    M_PRIV *const p = phase->priv;
    if (p->status == PS_ACTIVE) {
        Screenshot_Make(g_Config.rendering.screenshot_format);
        Sound_Effect(SFX_MENU_CHOOSE, NULL, SPM_ALWAYS);
        p->status = PS_COOLDOWN;
    } else if (p->status == PS_COOLDOWN) {
        p->status = PS_NONE;
    }

    Input_Update();
    Shell_ProcessInput();

    if (g_InputDB.toggle_ui) {
        UI_ToggleState(&g_Config.ui.enable_photo_mode_ui);
    }

    if (g_InputDB.toggle_photo_mode || g_InputDB.option) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_NOOP },
        };
    } else if (g_InputDB.action) {
        p->status = PS_ACTIVE;
    } else {
        p->ui->control(p->ui);
        Camera_Update();
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    const M_PRIV *const p = phase->priv;
    Interpolation_Disable();
    Game_Draw(false);
    Interpolation_Enable();
    if (p->status == PS_NONE) {
        p->ui->draw(p->ui);
    }
}

PHASE *Phase_PhotoMode_Create(void)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    phase->priv = Memory_Alloc(sizeof(M_PRIV));
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_PhotoMode_Destroy(PHASE *phase)
{
    Memory_Free(phase->priv);
    Memory_Free(phase);
}
