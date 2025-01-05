#include "game/phase/phase_pause.h"

#include "game/game.h"
#include "game/game_string.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/requester.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/memory.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAUSE_MAX_ITEMS 5
#define PAUSE_NUM_ITEM_TEXTS 2

typedef enum {
    STATE_DEFAULT,
    STATE_ASK,
    STATE_CONFIRM,
} STATE;

typedef struct {
    STATE state;
    bool is_text_ready;
    TEXTSTRING *mode_text;
    REQUEST_INFO requester;
} M_PRIV;

static void M_RemoveText(M_PRIV *p);
static void M_UpdateText(M_PRIV *p);
static int32_t M_DisplayRequester(
    M_PRIV *p, const char *header, const char *option1, const char *option2,
    int16_t requested);

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
    const char *option2, int16_t requested)
{
    if (!p->is_text_ready) {
        Requester_ClearTextstrings(&p->requester);
        Requester_SetSize(&p->requester, 2, -48);
        p->requester.requested = requested;
        Requester_SetHeading(&p->requester, header);
        Requester_AddItem(&p->requester, false, "%s", option1);
        Requester_AddItem(&p->requester, false, "%s", option2);
        p->is_text_ready = true;
        g_InputDB = (INPUT_STATE) { 0 };
        g_Input = (INPUT_STATE) { 0 };
    }

    // Don't allow menu_back because it clears the requester text.
    // The player must use the pause requester options to quit or continue.
    if (g_InputDB.menu_back) {
        g_InputDB = (INPUT_STATE) { 0 };
        g_Input = (INPUT_STATE) { 0 };
    }

    int select = Requester_Display(&p->requester);
    if (select > 0) {
        p->is_text_ready = false;
    } else {
        g_InputDB = (INPUT_STATE) { 0 };
        g_Input = (INPUT_STATE) { 0 };
    }
    return select;
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

    p->is_text_ready = false;
    p->mode_text = NULL;
    p->requester.items_used = 0;
    p->requester.max_items = PAUSE_NUM_ITEM_TEXTS;
    p->requester.requested = 0;
    p->requester.vis_lines = 0;
    p->requester.line_offset = 0;
    p->requester.line_old_offset = 0;
    p->requester.pix_width = 160;
    p->requester.line_height = TEXT_HEIGHT + 7;
    p->requester.is_blockable = false;
    p->requester.x = 0;
    p->requester.y = 0;
    p->requester.heading_text = NULL;
    p->requester.items = NULL;
    Requester_Init(&p->requester, PAUSE_NUM_ITEM_TEXTS);
    p->state = STATE_DEFAULT;
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Output_FadeToTransparent(true);
    M_RemoveText(p);
    Requester_Shutdown(&p->requester);
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t const num_frames)
{
    M_PRIV *const p = phase->priv;
    M_UpdateText(p);

    Input_Update();
    Shell_ProcessInput();
    Game_ProcessInput();

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
        int32_t choice = M_DisplayRequester(
            p, GS(PAUSE_EXIT_TO_TITLE), GS(PAUSE_CONTINUE), GS(PAUSE_QUIT), 1);
        if (choice == 1) {
            Music_Unpause();
            Sound_UnpauseAll();
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        } else if (choice == 2) {
            p->state = STATE_CONFIRM;
        }
        break;
    }

    case STATE_CONFIRM: {
        int32_t choice = M_DisplayRequester(
            p, GS(PAUSE_ARE_YOU_SURE), GS(PAUSE_YES), GS(PAUSE_NO), 1);
        if (choice == 1) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_TO_TITLE },
            };
        } else if (choice == 2) {
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
    Interpolation_Disable();
    Game_DrawScene(false);
    Interpolation_Enable();
    Output_AnimateFades();
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
