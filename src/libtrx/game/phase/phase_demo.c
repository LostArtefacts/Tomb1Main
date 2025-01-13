#include "game/phase/phase_demo.h"

#include "game/console/common.h"
#include "game/demo.h"
#include "game/fader.h"
#include "game/game.h"
#include "game/input.h"
#include "game/inventory_ring.h"
#include "game/output.h"
#include "game/text.h"
#include "memory.h"

typedef enum {
    STATE_RUN,
    STATE_FADE_OUT,
    STATE_FINISH,
} STATE;

typedef struct {
    STATE state;
    int32_t level_num;
    FADER top_fader;
    FADER exit_fader;
} M_PRIV;

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static void M_Suspend(PHASE *const phase);
static void M_Resume(PHASE *const phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (p->level_num == -1) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }

    if (!Demo_Start(p->level_num)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }

    p->state = STATE_RUN;
    g_OldInputDB = g_Input;
    Game_SetIsPlaying(true);

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    Demo_End();
}

static void M_Suspend(PHASE *const phase)
{
    Game_SetIsPlaying(false);
    Demo_Pause();
}

static void M_Resume(PHASE *const phase)
{
    Game_SetIsPlaying(true);
    Demo_Unpause();
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    switch (p->state) {
    case STATE_RUN:
        for (int32_t i = 0; i < num_frames; i++) {
            const GAME_FLOW_COMMAND gf_cmd = Demo_Control();
            if (gf_cmd.action != GF_NOOP) {
                p->state = STATE_FADE_OUT;
                Fader_Init(&p->top_fader, FADER_ANY, FADER_BLACK, 0.5);
                break;
            }
        }

        if (Game_IsExiting() && !Fader_IsActive(&p->exit_fader)) {
            p->state = STATE_FADE_OUT;
            Fader_Init(&p->exit_fader, FADER_TRANSPARENT, FADER_BLACK, 0.5);
        }

        break;

    case STATE_FADE_OUT:
        Game_SetIsPlaying(false);
        Demo_StopFlashing();
        if (!Fader_IsActive(&p->top_fader) && !Fader_IsActive(&p->exit_fader)) {
            p->state = STATE_FINISH;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_FINISH:
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = Game_IsExiting() ? GF_EXIT_GAME
                                                   : GF_EXIT_TO_TITLE },
        };
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Game_Draw(true);
    Text_Draw();
    Fader_Draw(&p->top_fader);
    Output_DrawPolyList();

    Console_Draw();
    Fader_Draw(&p->exit_fader);
    Output_DrawPolyList();
}

PHASE *Phase_Demo_Create(const int32_t level_num)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->level_num = level_num;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->suspend = M_Suspend;
    phase->resume = M_Resume;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Demo_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
