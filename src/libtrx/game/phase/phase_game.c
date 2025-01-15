#include "game/phase/phase_game.h"

#include "game/console/common.h"
#include "game/game.h"
#include "game/output.h"
#include "game/text.h"
#include "memory.h"

typedef struct {
    int32_t level_num;
    GAME_FLOW_LEVEL_TYPE level_type;
} M_PRIV;

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static void M_Resume(PHASE *const phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (!Game_Start(p->level_num, p->level_type)) {
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
    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
}

PHASE *Phase_Game_Create(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->level_num = level_num;
    p->level_type = level_type;
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
