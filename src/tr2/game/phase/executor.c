#include "game/phase/executor.h"

#include "game/clock.h"
#include "game/output.h"
#include "game/phase/priv.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/memory.h>

#include <stdbool.h>
#include <stddef.h>

static PHASE_CONTROL M_Control(PHASE *phase, int32_t nframes);
static void M_Draw(PHASE *phase);
static int32_t M_Wait(PHASE *phase);

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t nframes)
{
    if (g_GF_OverrideDir != (GAME_FLOW_DIR)-1) {
        const GAME_FLOW_DIR dir = g_GF_OverrideDir;
        g_GF_OverrideDir = -1;
        return (PHASE_CONTROL) { .action = PHASE_ACTION_END, .dir = dir };
    }
    if (phase != NULL && phase->control != NULL) {
        return phase->control(phase, nframes);
    }
    return (PHASE_CONTROL) {
        .action = PHASE_ACTION_END,
        .dir = (GAME_FLOW_DIR)-1,
    };
}

static void M_Draw(PHASE *const phase)
{
    Output_BeginScene();
    if (phase != NULL && phase->draw != NULL) {
        phase->draw(phase);
    }
    Output_EndScene(false);
}

static int32_t M_Wait(PHASE *const phase)
{
    if (phase != NULL && phase->wait != NULL) {
        return phase->wait(phase);
    } else {
        return Clock_WaitTick();
    }
}

GAME_FLOW_DIR PhaseExecutor_Run(PHASE *const phase)
{
    PHASE_CONTROL control;

    if (phase->start != NULL) {
        control = phase->start(phase);
        if (control.action == PHASE_ACTION_END) {
            return control.dir;
        } else if (g_GF_OverrideDir != (GAME_FLOW_DIR)-1) {
            const GAME_FLOW_DIR dir = g_GF_OverrideDir;
            g_GF_OverrideDir = -1;
            return dir;
        } else if (g_IsGameToExit) {
            return GFD_EXIT_GAME;
        }
    }

    int32_t nframes = Clock_WaitTick();
    while (true) {
        control = M_Control(phase, nframes);

        M_Draw(phase);
        if (control.action == PHASE_ACTION_END) {
            break;
        } else if (control.action == PHASE_ACTION_NO_WAIT) {
            nframes = 0;
            continue;
        } else {
            nframes = M_Wait(phase);
        }
    }

    if (phase->end != NULL) {
        phase->end(phase);
    }

    if (g_IsGameToExit) {
        return GFD_EXIT_GAME;
    }

    return control.dir;
}
