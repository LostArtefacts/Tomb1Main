#include "game/phase/phase_cutscene.h"

#include "game/cutscene.h"
#include "game/game.h"

#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef struct {
    int32_t level_num;
    bool exiting;
    FADER exit_fader;
} M_PRIV;

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static void M_Suspend(PHASE *phase);
static void M_Resume(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Cutscene_Start(p->level_num);
    Game_SetIsPlaying(true);
    return (PHASE_CONTROL) {};
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Game_SetIsPlaying(false);
    Cutscene_End();
}

static void M_Suspend(PHASE *const phase)
{
    Game_SetIsPlaying(false);
}

static void M_Resume(PHASE *const phase)
{
    Game_SetIsPlaying(true);
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    if (Game_IsExiting() && !p->exiting) {
        p->exiting = true;
        Fader_Init(&p->exit_fader, FADER_ANY, FADER_BLACK, 1.0 / 3.0);
        return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
    } else if (p->exiting && !Fader_IsActive(&p->exit_fader)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_GAME },
        };
    } else {
        for (int32_t i = 0; i < num_frames; i++) {
            const GAME_FLOW_COMMAND gf_cmd = Cutscene_Control();
            if (gf_cmd.action != GF_NOOP) {
                return (PHASE_CONTROL) {
                    .action = PHASE_ACTION_END,
                    .gf_cmd = gf_cmd,
                };
            }
        }
        return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
    }
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Cutscene_Draw();
    Fader_Draw(&p->exit_fader);
}

PHASE *Phase_Cutscene_Create(const int32_t level_num)
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

void Phase_Cutscene_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
