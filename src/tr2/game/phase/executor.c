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
    if (g_GF_OverrideCommand.action != GF_NOOP) {
        const GAME_FLOW_COMMAND gf_cmd = g_GF_OverrideCommand;
        g_GF_OverrideCommand = (GAME_FLOW_COMMAND) { .action = GF_NOOP };
        return (PHASE_CONTROL) { .action = PHASE_ACTION_END, .gf_cmd = gf_cmd };
    }
    if (phase != NULL && phase->control != NULL) {
        return phase->control(phase, nframes);
    }
    return (PHASE_CONTROL) {
        .action = PHASE_ACTION_END,
        .gf_cmd = { .action = GF_NOOP },
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

GAME_FLOW_COMMAND PhaseExecutor_Run(PHASE *const phase)
{
    GAME_FLOW_COMMAND gf_cmd = { .action = GF_NOOP };

    PHASE *const prev_phase =
        m_PhaseStackSize > 0 ? m_PhaseStack[m_PhaseStackSize - 1] : NULL;
    if (prev_phase != NULL && prev_phase->suspend != NULL) {
        prev_phase->suspend(phase);
    }
    m_PhaseStack[m_PhaseStackSize++] = phase;

    if (phase->start != NULL) {
        Clock_SyncTick();
        const PHASE_CONTROL control = phase->start(phase);
        if (g_IsGameToExit) {
            gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
            goto finish;
        } else if (control.action == PHASE_ACTION_END) {
            gf_cmd = control.gf_cmd;
            goto finish;
        }
    }

    int32_t nframes = Clock_WaitTick();
    while (true) {
        const PHASE_CONTROL control = M_Control(phase, nframes);

        if (control.action == PHASE_ACTION_END) {
            if (g_IsGameToExit) {
                gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
            } else {
                gf_cmd = control.gf_cmd;
            }
            goto finish;
        } else if (control.action == PHASE_ACTION_NO_WAIT) {
            nframes = 0;
            continue;
        } else {
            M_Draw(phase);
            nframes = M_Wait(phase);
        }
    }

finish:
    if (phase->end != NULL) {
        phase->end(phase);
    }
    if (prev_phase != NULL && prev_phase->resume != NULL) {
        Clock_SyncTick();
        prev_phase->resume(phase);
    }
    m_PhaseStackSize--;

    return gf_cmd;
}
