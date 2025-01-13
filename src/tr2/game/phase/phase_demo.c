#include "game/phase/phase_demo.h"

#include "game/console/common.h"
#include "game/demo.h"
#include "game/game.h"
#include "game/input.h"
#include "game/inventory_ring.h"
#include "game/output.h"

#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef struct {
    bool exiting;
    int32_t level_num;
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
    Fader_Init(&p->exit_fader, FADER_TRANSPARENT, FADER_TRANSPARENT, 0.0);

    g_OldInputDB = g_Input;
    Game_SetIsPlaying(true);

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    Game_SetIsPlaying(false);
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
    if (Game_IsExiting() && !p->exiting) {
        p->exiting = true;
        Fader_Init(&p->exit_fader, FADER_ANY, FADER_BLACK, 0.5);
    } else if (p->exiting && !Fader_IsActive(&p->exit_fader)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_GAME },
        };
    } else {
        for (int32_t i = 0; i < num_frames; i++) {
            const GAME_FLOW_COMMAND gf_cmd = Demo_Control();
            if (gf_cmd.action != GF_NOOP) {
                return (PHASE_CONTROL) {
                    .action = PHASE_ACTION_END,
                    .gf_cmd = gf_cmd,
                };
            }
        }
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Game_Draw(true);
    Output_DrawPolyList();
    Console_Draw();
    Text_Draw();
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
