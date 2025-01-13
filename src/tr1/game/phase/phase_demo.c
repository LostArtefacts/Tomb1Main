#include "game/phase/phase_demo.h"

#include "game/demo.h"
#include "game/game.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/phase.h"
#include "game/shell.h"
#include "game/text.h"

#include <libtrx/debug.h>
#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef enum {
    STATE_RUN,
    STATE_FADE_OUT,
} STATE;

typedef struct {
    STATE state;
    int32_t demo_num;
    int32_t level_num;
    FADER fader;
} M_PRIV;

static void M_FadeOut(M_PRIV *const p);

static PHASE_CONTROL M_ControlRun(M_PRIV *p, int32_t num_frames);
static PHASE_CONTROL M_ControlFadeOut(M_PRIV *p);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static void M_Suspend(PHASE *phase);
static void M_Resume(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t num_frames);
static void M_Draw(PHASE *phase);

static void M_FadeOut(M_PRIV *const p)
{
    Demo_StopFlashing();
    p->state = STATE_FADE_OUT;
    Fader_Init(&p->fader, FADER_TRANSPARENT, FADER_BLACK, 0.5);
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    p->level_num = Demo_ChooseLevel(p->demo_num);
    if (p->level_num == -1) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }

    p->state = STATE_RUN;

    Fader_Init(&p->fader, FADER_TRANSPARENT, FADER_TRANSPARENT, 0.0);
    if (!Demo_Start(p->level_num)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }
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

static PHASE_CONTROL M_ControlRun(M_PRIV *const p, const int32_t num_frames)
{
    Interpolation_Remember();
    for (int32_t i = 0; i < num_frames; i++) {
        const GAME_FLOW_COMMAND gf_cmd = Demo_Control();
        if (gf_cmd.action == GF_LEVEL_COMPLETE) {
            M_FadeOut(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
    }

    // Discard demo input; check for debounced real keypresses
    Input_Update();
    Shell_ProcessInput();
    if (g_InputDB.pause) {
        Game_SetIsPlaying(false);
        PHASE *const subphase = Phase_Pause_Create();
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
        Phase_Pause_Destroy(subphase);
        Game_SetIsPlaying(true);
        if (gf_cmd.action != GF_NOOP) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = gf_cmd,
            };
        }
        return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
    } else if (g_InputDB.toggle_photo_mode) {
        Game_SetIsPlaying(false);
        PHASE *const subphase = Phase_PhotoMode_Create();
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
        Phase_PhotoMode_Destroy(subphase);
        Game_SetIsPlaying(true);
        if (gf_cmd.action != GF_NOOP) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = gf_cmd,
            };
        }
        return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
    } else if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
        M_FadeOut(p);
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static PHASE_CONTROL M_ControlFadeOut(M_PRIV *const p)
{
    Input_Update();
    Shell_ProcessInput();

    if (g_InputDB.menu_confirm || g_InputDB.menu_back
        || !Fader_IsActive(&p->fader)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    switch (p->state) {
    case STATE_RUN:
        return M_ControlRun(p, num_frames);

    case STATE_FADE_OUT:
        return M_ControlFadeOut(p);
    }

    ASSERT_FAIL();
    return (PHASE_CONTROL) {
        .action = PHASE_ACTION_END,
        .gf_cmd = { .action = GF_NOOP },
    };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (p->state == STATE_FADE_OUT) {
        Interpolation_Disable();
    }
    Game_Draw(true);
    if (p->state == STATE_FADE_OUT) {
        Interpolation_Enable();
    }
    Output_DrawPolyList();

    Text_Draw();
    Fader_Draw(&p->fader);
    Output_DrawPolyList();
}

PHASE *Phase_Demo_Create(const int32_t demo_num)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->demo_num = demo_num;
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
