#include "game/phase/phase_game.h"

#include "game/game.h"
#include "game/output.h"
#include "memory.h"

typedef struct {
    const GAME_FLOW_LEVEL *level;
    GAME_FLOW_SEQUENCE_CONTEXT seq_ctx;
} M_PRIV;

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static void M_Resume(PHASE *const phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (!Game_Start(p->level, p->seq_ctx)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }
    Game_SetIsPlaying(true);
    return (PHASE_CONTROL) {
        .action = PHASE_ACTION_CONTINUE,
    };
}

static void M_End(PHASE *const phase)
{
    Game_End();
    Game_SetIsPlaying(false);
}

static void M_Suspend(PHASE *const phase)
{
    Game_Suspend();
    Game_SetIsPlaying(false);
}

static void M_Resume(PHASE *const phase)
{
    Game_Resume();
    Game_SetIsPlaying(true);
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    for (int32_t i = 0; i < num_frames; i++) {
        const GAME_FLOW_COMMAND gf_cmd = Game_Control(false);
        if (gf_cmd.action != GF_NOOP) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = gf_cmd,
            };
        }
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    Game_Draw(true);
}

PHASE *Phase_Game_Create(
    const GAME_FLOW_LEVEL *const level,
    const GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->level = level;
    p->seq_ctx = seq_ctx;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->suspend = M_Suspend;
    phase->resume = M_Resume;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Game_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
