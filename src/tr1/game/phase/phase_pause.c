#include "game/phase/phase_pause.h"

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
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/game/fader.h>
#include <libtrx/game/ui/widgets/requester.h>
#include <libtrx/memory.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    STATE_DEFAULT,
    STATE_ASK,
    STATE_CONFIRM,
    STATE_FADE_OUT,
} STATE;

typedef struct {
    STATE state;
    bool is_ui_ready;
    UI_WIDGET *ui;
    TEXTSTRING *mode_text;
    FADER back_fader;
} M_PRIV;

static void M_FadeIn(M_PRIV *p);
static void M_FadeOut(M_PRIV *p);
static void M_PauseGame(M_PRIV *p);
static void M_ReturnToGame(M_PRIV *p);
static void M_CreateText(M_PRIV *p);
static void M_RemoveText(M_PRIV *p);
static int32_t M_DisplayRequester(
    M_PRIV *p, const char *header, const char *option1, const char *option2);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t nframes);
static void M_Draw(PHASE *phase);

static void M_FadeIn(M_PRIV *const p)
{
    Fader_Init(&p->back_fader, FADER_TRANSPARENT, FADER_SEMI_BLACK, 0.5);
}

static void M_FadeOut(M_PRIV *const p)
{
    Fader_Init(&p->back_fader, FADER_ANY, FADER_TRANSPARENT, 0.3);
    p->state = STATE_FADE_OUT;
}

static void M_PauseGame(M_PRIV *const p)
{
    Music_Pause();
    Sound_PauseAll();
    Overlay_HideGameInfo();
    M_CreateText(p);
    M_FadeIn(p);
}

static void M_ReturnToGame(M_PRIV *const p)
{
    Music_Unpause();
    Sound_UnpauseAll();
    M_RemoveText(p);
    if (p->ui != NULL) {
        p->ui->free(p->ui);
        p->ui = NULL;
    }
    M_FadeOut(p);
}

static void M_CreateText(M_PRIV *const p)
{
    if (p->mode_text == NULL) {
        p->mode_text = Text_Create(0, -24, GS(PAUSE_PAUSED));
        Text_CentreH(p->mode_text, true);
        Text_AlignBottom(p->mode_text, true);
    }
}

static void M_RemoveText(M_PRIV *const p)
{
    Text_Remove(p->mode_text);
    p->mode_text = NULL;
}

static int32_t M_DisplayRequester(
    M_PRIV *const p, const char *header, const char *option1,
    const char *option2)
{
    if (!p->is_ui_ready) {
        if (p->ui == NULL) {
            p->ui = UI_Requester_Create((UI_REQUESTER_SETTINGS) {
                .is_selectable = true,
                .width = 160,
                .visible_rows = 2,
            });
        }
        UI_Requester_ClearRows(p->ui);
        UI_Requester_SetTitle(p->ui, header);
        UI_Requester_AddRowC(p->ui, option1, NULL);
        UI_Requester_AddRowC(p->ui, option2, NULL);
        p->is_ui_ready = true;
    }

    const int32_t choice = UI_Requester_GetSelectedRow(p->ui);
    if (choice >= 0) {
        p->is_ui_ready = false;
    }
    return choice;
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    g_OldInputDB = g_Input;
    Output_SetupAboveWater(false);

    M_PauseGame(p);

    p->is_ui_ready = false;
    p->state = STATE_DEFAULT;
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    M_RemoveText(p);
    if (p->ui != NULL) {
        p->ui->free(p->ui);
        p->ui = NULL;
    }
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t const num_frames)
{
    M_PRIV *const p = phase->priv;

    Input_Update();
    Shell_ProcessInput();

    if (p->ui != NULL) {
        p->ui->control(p->ui);
    }

    switch (p->state) {
    case STATE_DEFAULT:
        if (g_InputDB.pause) {
            M_ReturnToGame(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (g_InputDB.option) {
            p->state = STATE_ASK;
        }
        break;

    case STATE_ASK: {
        const int32_t choice = M_DisplayRequester(
            p, GS(PAUSE_EXIT_TO_TITLE), GS(PAUSE_CONTINUE), GS(PAUSE_QUIT));
        if (choice == 0) {
            M_ReturnToGame(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (choice == 1) {
            p->state = STATE_CONFIRM;
        }
        break;
    }

    case STATE_CONFIRM: {
        const int32_t choice = M_DisplayRequester(
            p, GS(PAUSE_ARE_YOU_SURE), GS(PAUSE_YES), GS(PAUSE_NO));
        if (choice == 0) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_TO_TITLE },
            };
        } else if (choice == 1) {
            M_ReturnToGame(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_FADE_OUT:
        if (!Fader_IsActive(&p->back_fader)) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }
        break;
    }
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Interpolation_Disable();
    Game_Draw(false);
    Fader_Draw(&p->back_fader);
    Interpolation_Enable();
    if (p->ui != NULL) {
        p->ui->draw(p->ui);
    }
    Text_Draw();
}

PHASE *Phase_Pause_Create(void)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    phase->priv = Memory_Alloc(sizeof(M_PRIV));
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Pause_Destroy(PHASE *phase)
{
    Memory_Free(phase->priv);
    Memory_Free(phase);
}
