#include "game/phase/phase_stats.h"

#include "game/console/common.h"
#include "game/input.h"
#include "game/music.h"
#include "game/output.h"
#include "game/stats.h"
#include "game/ui/widgets/stats_dialog.h"
#include "global/vars.h"

#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef enum {
    STATE_FADE_IN,
    STATE_WAIT,
    STATE_FADE_OUT,
} M_STATE;

typedef struct {
    M_STATE state;
    FADER fader;
    UI_WIDGET *dialog;
    PHASE_STATS_ARGS args;
} M_PRIV;

static void M_FadeOut(M_PRIV *p);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static void M_FadeOut(M_PRIV *const p)
{
    p->state = STATE_FADE_OUT;
    Fader_Init(&p->fader, FADER_ANY, FADER_BLACK, p->args.fade_out_time);
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (p->args.show_final_stats) {
        Output_LoadBackgroundFromFile("data/end.pcx");
    } else {
        Music_Play(g_GameFlow.level_complete_track, MPM_ALWAYS);
        Output_LoadBackgroundFromObject();
    }
    p->dialog = UI_StatsDialog_Create(
        p->args.show_final_stats ? UI_STATS_DIALOG_MODE_FINAL
                                 : UI_STATS_DIALOG_MODE_LEVEL);
    Fader_Init(&p->fader, FADER_BLACK, FADER_TRANSPARENT, p->args.fade_in_time);
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    p->dialog->free(p->dialog);
    Output_UnloadBackground();
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;
    Input_Update();

    switch (p->state) {
    case STATE_FADE_IN:
        if (g_InputDB.menu_confirm || g_InputDB.menu_back || g_IsGameToExit) {
            M_FadeOut(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (!Fader_IsActive(&p->fader)) {
            p->state = STATE_WAIT;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_WAIT:
        if (g_InputDB.menu_confirm || g_InputDB.menu_back || g_IsGameToExit) {
            M_FadeOut(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_FADE_OUT:
        if (g_InputDB.menu_confirm || g_InputDB.menu_back
            || !Fader_IsActive(&p->fader)) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }
    }

    p->dialog->control(p->dialog);
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Output_DrawBackground();
    p->dialog->draw(p->dialog);
    Text_Draw();
    Output_DrawPolyList();
    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->fader));
    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
}

PHASE *Phase_Stats_Create(const PHASE_STATS_ARGS args)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->args = args;
    p->state = STATE_FADE_IN;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Stats_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
