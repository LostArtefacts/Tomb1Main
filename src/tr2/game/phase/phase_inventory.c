#include "game/phase/phase_inventory.h"

#include "game/console/common.h"
#include "game/input.h"
#include "game/inventory_ring.h"
#include "game/output.h"
#include "game/overlay.h"
#include "global/vars.h"

#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef enum {
    STATE_FADE_IN,
    STATE_RUN,
    STATE_FADE_OUT,
    STATE_EXIT,
} M_STATE;

typedef struct {
    M_STATE state;
    INVENTORY_MODE mode;
    INV_RING *ring;
    FADER fader;
    GAME_FLOW_COMMAND gf_cmd;
} M_PRIV;

static void M_FadeOut(M_PRIV *p);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static void M_FadeOut(M_PRIV *const p)
{
    if (p->ring->mode == INV_TITLE_MODE) {
        p->state = STATE_FADE_OUT;
        Fader_InitAnyToBlack(&p->fader, FRAMES_PER_SECOND / 3);
    } else {
        p->state = STATE_EXIT;
    }
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (p->mode == INV_TITLE_MODE) {
        Fader_InitBlackToTransparent(&p->fader, FRAMES_PER_SECOND / 3);
    } else {
        Fader_InitBlackToTransparent(&p->fader, 0);
    }
    p->ring = InvRing_Open(p->mode);
    if (p->ring == NULL) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }
    return (PHASE_CONTROL) {};
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    // g_GF_OverrideCommand from PhaseExecutor_Run caused the loop to end
    // early, bypassing STATE_EXIT. Need to clean up manually.
    if (p->ring != NULL) {
        InvRing_Close(p->ring);
        p->ring = NULL;
    }
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    switch (p->state) {
    case STATE_FADE_IN:
        Input_Update();
        if (!Fader_Control(&p->fader)) {
            p->state = STATE_RUN;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_RUN: {
        const GAME_FLOW_COMMAND gf_cmd = InvRing_Control(p->ring, num_frames);
        if (gf_cmd.action != GF_NOOP || p->ring->motion.status == RNG_DONE) {
            p->gf_cmd = gf_cmd;
            M_FadeOut(p);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;
    }

    case STATE_FADE_OUT:
        Input_Update();
        if (g_InputDB.menu_confirm || g_InputDB.menu_back
            || !Fader_Control(&p->fader)) {
            p->state = STATE_EXIT;
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        }
        break;

    case STATE_EXIT:
        GAME_FLOW_COMMAND gf_cmd = InvRing_Close(p->ring);
        if (gf_cmd.action == GF_NOOP) {
            gf_cmd = p->gf_cmd;
        }
        p->ring = NULL;
        return (PHASE_CONTROL) { .action = PHASE_ACTION_END, .gf_cmd = gf_cmd };
    }

    return (PHASE_CONTROL) {};
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Output_DrawBackground();
    if (p->ring != NULL && p->state != STATE_FADE_IN) {
        InvRing_Draw(p->ring);
    }

    Overlay_DrawModeInfo();
    Text_Draw();
    Output_DrawPolyList();

    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->fader));

    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
}

PHASE *Phase_Inventory_Create(const INVENTORY_MODE mode)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->mode = mode;
    p->state = mode == INV_TITLE_MODE ? STATE_FADE_IN : STATE_RUN;
    p->gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Inventory_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
