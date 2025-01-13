#include "game/phase/phase_photo_mode.h"

#include "config.h"
#include "game/camera.h"
#include "game/console/common.h"
#include "game/fader.h"
#include "game/game.h"
#include "game/game_string.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/text.h"
#include "game/ui/common.h"
#include "game/ui/widgets/photo_mode.h"
#include "memory.h"

#include <stdio.h>

typedef enum {
    STATE_NAVIGATE,
    STATE_FADE_OUT,
    STATE_FINISH,
} STATE;

typedef struct {
    STATE state;
    UI_WIDGET *ui;
    FADER top_fader;
    bool taking_screenshot;
    bool show_fps_counter;
} M_PRIV;

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t num_frames);
static void M_Draw(PHASE *phase);

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    p->state = STATE_NAVIGATE;
#if TR_VERSION == 1
    p->show_fps_counter = g_Config.rendering.enable_fps_counter;
    g_Config.rendering.enable_fps_counter = false;
#endif

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

#if TR_VERSION == 1
    g_Config.rendering.enable_fps_counter = p->show_fps_counter;
#endif
    g_Input = g_OldInputDB;
    Music_Unpause();
    Sound_UnpauseAll();
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t num_frames)
{
    M_PRIV *const p = phase->priv;
    switch (p->state) {
    case STATE_NAVIGATE:
        Input_Update();
        Shell_ProcessInput();

        if (g_InputDB.toggle_ui) {
            UI_ToggleState(&g_Config.ui.enable_photo_mode_ui);
        }

        if (g_InputDB.toggle_photo_mode || g_InputDB.menu_back) {
            p->state = STATE_FINISH;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (Game_IsExiting()) {
            Fader_Init(&p->top_fader, FADER_TRANSPARENT, FADER_BLACK, 0.5);
            p->state = STATE_FADE_OUT;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (g_InputDB.action) {
            Output_BeginScene();
            p->taking_screenshot = true;
            M_Draw(phase);
            p->taking_screenshot = false;
            Screenshot_Make(g_Config.rendering.screenshot_format);
            Output_EndScene();
            Sound_Effect(SFX_MENU_LARA_HOME, NULL, SPM_ALWAYS);
        } else {
            p->ui->control(p->ui);
            Camera_Update();
        }
        break;

    case STATE_FADE_OUT:
        if (g_InputDB.menu_confirm || g_InputDB.menu_back
            || !Fader_IsActive(&p->top_fader)) {
            p->state = STATE_FINISH;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_FINISH:
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = Game_IsExiting() ? GF_EXIT_GAME : GF_NOOP },
        };
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Interpolation_Disable();
    Game_Draw(false);
    Interpolation_Enable();
    Output_DrawPolyList();

    if (!p->taking_screenshot) {
        p->ui->draw(p->ui);
    }
    Fader_Draw(&p->top_fader);

    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
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
