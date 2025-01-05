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

#include <libtrx/game/ui/widgets/requester.h>
#include <libtrx/memory.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    STATE_DEFAULT,
    STATE_ASK,
    STATE_CONFIRM,
} STATE;

typedef struct {
    STATE state;
    bool is_ready;
    TEXTSTRING *mode_text;
    UI_WIDGET *ui;
} M_PRIV;

static void M_RemoveText(M_PRIV *p);
static void M_UpdateText(M_PRIV *p);
static int32_t M_DisplayRequester(
    M_PRIV *p, const char *header, const char *option1, const char *option2);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t nframes);
static void M_Draw(PHASE *phase);

static void M_RemoveText(M_PRIV *const p)
{
    Text_Remove(p->mode_text);
    p->mode_text = NULL;
}

static void M_UpdateText(M_PRIV *const p)
{
    if (p->mode_text == NULL) {
        p->mode_text = Text_Create(0, -24, GS(PAUSE_PAUSED));
        Text_CentreH(p->mode_text, true);
        Text_AlignBottom(p->mode_text, true);
    }
}

static int32_t M_DisplayRequester(
    M_PRIV *const p, const char *header, const char *option1,
    const char *option2)
{
    if (!p->is_ready) {
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
        p->is_ready = true;
    }

    const int32_t choice = UI_Requester_GetSelectedRow(p->ui);
    if (choice >= 0) {
        p->is_ready = false;
    }
    return choice;
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    g_OldInputDB = g_Input;
    Overlay_HideGameInfo();
    Output_SetupAboveWater(false);
    Music_Pause();
    Sound_PauseAll();
    Output_FadeToSemiBlack(true);

    p->is_ready = false;
    p->mode_text = NULL;
    p->state = STATE_DEFAULT;
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Output_FadeToTransparent(true);
    M_RemoveText(p);
    if (p->ui != NULL) {
        p->ui->free(p->ui);
    }
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t const num_frames)
{
    M_PRIV *const p = phase->priv;
    M_UpdateText(p);

    Input_Update();
    Shell_ProcessInput();

    if (p->ui != NULL) {
        p->ui->control(p->ui);
    }

    switch (p->state) {
    case STATE_DEFAULT:
        if (g_InputDB.pause) {
            Music_Unpause();
            Sound_UnpauseAll();
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        } else if (g_InputDB.option) {
            p->state = STATE_ASK;
        }
        break;

    case STATE_ASK: {
        const int32_t choice = M_DisplayRequester(
            p, GS(PAUSE_EXIT_TO_TITLE), GS(PAUSE_CONTINUE), GS(PAUSE_QUIT));
        if (choice == 0) {
            Music_Unpause();
            Sound_UnpauseAll();
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
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
            Music_Unpause();
            Sound_UnpauseAll();
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
    Game_DrawScene(false);
    Interpolation_Enable();
    Output_AnimateFades();
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
