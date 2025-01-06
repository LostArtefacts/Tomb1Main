#include "game/phase/phase_inventory.h"

#include "game/input.h"
#include "game/interpolation.h"
#include "game/inventory_ring.h"
#include "game/music.h"
#include "game/option.h"
#include "game/output.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/inventory_ring/priv.h>

static INV_RING *m_Ring;

static void M_Start(const PHASE_INVENTORY_ARGS *args);
static void M_End(void);
static PHASE_CONTROL M_ControlFrame(void);
static PHASE_CONTROL M_Control(int32_t nframes);
static void M_Draw(void);

static void M_Start(const PHASE_INVENTORY_ARGS *const args)
{
    Interpolation_Remember();
    if (g_Config.gameplay.enable_timer_in_inventory) {
        Stats_StartTimer();
    }

    m_Ring = InvRing_Open(args->mode);
    g_InvMode = args->mode;

    if (g_InvMode == INV_TITLE_MODE) {
        Output_FadeResetToBlack();
        Output_FadeToTransparent(true);
    } else {
        Output_FadeToSemiBlack(true);
    }
}

static PHASE_CONTROL M_Control(int32_t num_frames)
{
    Interpolation_Remember();
    if (g_Config.gameplay.enable_timer_in_inventory) {
        Stats_UpdateTimer();
    }
    if (m_Ring != NULL) {
        PHASE_CONTROL result = InvRing_Control(m_Ring, num_frames);
        if (result.action == PHASE_ACTION_END) {
            INV_RING *const old_ring = m_Ring;
            m_Ring = NULL;
            result = InvRing_Close(old_ring);
        }
        return result;
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_END };
}

static void M_End(void)
{
    if (m_Ring != NULL) {
        INV_RING *const old_ring = m_Ring;
        m_Ring = NULL;
        InvRing_Close(old_ring);
    }
}

static void M_Draw(void)
{
    if (m_Ring != NULL) {
        InvRing_Draw(m_Ring);
    }
    Output_AnimateFades();
    Text_Draw();
}

PHASER g_InventoryPhaser = {
    .start = (PHASER_START)M_Start,
    .end = M_End,
    .control = M_Control,
    .draw = M_Draw,
    .wait = NULL,
};
