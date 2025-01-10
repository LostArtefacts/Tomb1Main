#include "game/phase/phase_stats.h"

#include "game/console/common.h"
#include "game/game.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/shell.h"
#include "game/stats.h"
#include "game/ui/widgets/stats_dialog.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/memory.h>

typedef enum {
    STATE_FADE_IN,
    STATE_DISPLAY,
    STATE_FADE_OUT,
} STATE;

typedef struct {
    PHASE_STATS_ARGS args;
    STATE state;
    FADER back_fader;
    FADER top_fader;
    UI_WIDGET *ui;
} M_PRIV;

static bool M_IsFading(M_PRIV *p);
static void M_FadeIn(M_PRIV *p);
static void M_FadeOut(M_PRIV *p);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t num_frames);
static void M_Draw(PHASE *phase);

static bool M_IsFading(M_PRIV *const p)
{
    return Fader_IsActive(&p->top_fader) || Fader_IsActive(&p->back_fader);
}

static void M_FadeIn(M_PRIV *const p)
{
    if (p->args.background_path != NULL) {
        Fader_Init(&p->top_fader, FADER_BLACK, FADER_TRANSPARENT, 1.0);
    } else {
        Fader_Init(&p->back_fader, FADER_TRANSPARENT, FADER_SEMI_BLACK, 0.5);
    }
}

static void M_FadeOut(M_PRIV *const p)
{
    Fader_Init(&p->top_fader, FADER_ANY, FADER_BLACK, 0.5);
    p->state = STATE_FADE_OUT;
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    if (p->args.background_path != NULL) {
        Output_LoadBackgroundFromFile(p->args.background_path);
    } else {
        Output_UnloadBackground();
    }

    if (g_CurrentLevel == g_GameFlow.gym_level_num) {
        M_FadeOut(p);
    } else {
        M_FadeIn(p);

        p->ui = UI_StatsDialog_Create(
            p->args.show_final_stats ? UI_STATS_DIALOG_MODE_FINAL
                                     : UI_STATS_DIALOG_MODE_LEVEL,
            p->args.level_num != -1 ? p->args.level_num : g_CurrentLevel,
            p->args.level_type);
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (p->ui != NULL) {
        p->ui->free(p->ui);
    }
    Music_Stop();
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t num_frames)
{
    M_PRIV *const p = phase->priv;
    Input_Update();
    Shell_ProcessInput();

    switch (p->state) {
    case STATE_FADE_IN:
        if (!M_IsFading(p)) {
            p->state = STATE_DISPLAY;
        } else if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            M_FadeOut(p);
        }
        break;

    case STATE_DISPLAY:
        if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            M_FadeOut(p);
        }
        break;

    case STATE_FADE_OUT:
        M_FadeOut(p);
        if (g_InputDB.menu_confirm || g_InputDB.menu_back || !M_IsFading(p)) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }
        break;
    }

    if (p->ui != NULL) {
        p->ui->control(p->ui);
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (!p->args.show_final_stats) {
        Interpolation_Disable();
        Game_Draw(false);
        Interpolation_Enable();
        Fader_Draw(&p->back_fader);
    }
    if (p->ui != NULL) {
        p->ui->draw(p->ui);
    }
    Text_Draw();
    Output_DrawPolyList();
    Fader_Draw(&p->top_fader);
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
