#include "game/phase/executor.h"

#include "game/clock.h"
#include "game/output.h"
#include "game/phase/priv.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/memory.h>

#include <stdbool.h>
#include <stddef.h>

#define MAX_PHASES 10
static int32_t m_PhaseStackSize = 0;
static PHASE *m_PhaseStack[MAX_PHASES] = {};

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
    GAME_FLOW_DIR result = (GAME_FLOW_DIR)-1;

    PHASE *const prev_phase =
        m_PhaseStackSize > 0 ? m_PhaseStack[m_PhaseStackSize - 1] : NULL;
    if (prev_phase != NULL && prev_phase->suspend != NULL) {
        prev_phase->suspend(phase);
    }
    m_PhaseStack[m_PhaseStackSize++] = phase;

    if (phase->start != NULL) {
        const PHASE_CONTROL control = phase->start(phase);
        if (g_IsGameToExit) {
            result = GFD_EXIT_GAME;
            goto finish;
        } else if (control.action == PHASE_ACTION_END) {
            result = control.dir;
            goto finish;
        }
    }

    int32_t nframes = Clock_WaitTick();
    while (true) {
        const PHASE_CONTROL control = M_Control(phase, nframes);

        M_Draw(phase);
        if (control.action == PHASE_ACTION_END) {
            if (g_IsGameToExit) {
                result = GFD_EXIT_GAME;
            } else {
                result = control.dir;
            }
            goto finish;
        } else if (control.action == PHASE_ACTION_NO_WAIT) {
            nframes = 0;
            continue;
        } else {
            nframes = M_Wait(phase);
        }
    }

finish:
    if (phase->end != NULL) {
        phase->end(phase);
    }
    if (prev_phase != NULL && prev_phase->resume != NULL) {
        prev_phase->resume(phase);
    }
    m_PhaseStackSize--;

    return result;
}
