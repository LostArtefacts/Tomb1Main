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

INV_RING m_Ring;

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

    INV_RING *ring = &m_Ring;
    memset(ring, 0, sizeof(INV_RING));

    g_InvMode = args->mode;

    InvRing_Construct();

    if (!g_Config.audio.enable_music_in_inventory
        && g_InvMode != INV_TITLE_MODE) {
        Music_Pause();
        Sound_PauseAll();
    } else {
        Sound_ResetAmbient();
        Sound_UpdateEffects();
    }

    switch (g_InvMode) {
    case INV_DEATH_MODE:
    case INV_SAVE_MODE:
    case INV_SAVE_CRYSTAL_MODE:
    case INV_LOAD_MODE:
    case INV_TITLE_MODE:
        InvRing_InitRing(
            ring, RT_OPTION, g_InvRing_Source[RT_OPTION].items,
            g_InvRing_Source[RT_OPTION].count,
            g_InvRing_Source[RT_OPTION].current);
        break;

    case INV_KEYS_MODE:
        InvRing_InitRing(
            ring, RT_KEYS, g_InvRing_Source[RT_KEYS].items,
            g_InvRing_Source[RT_KEYS].count, g_InvRing_Source[RT_MAIN].current);
        break;

    default:
        if (g_InvRing_Source[RT_MAIN].count != 0) {
            InvRing_InitRing(
                ring, RT_MAIN, g_InvRing_Source[RT_MAIN].items,
                g_InvRing_Source[RT_MAIN].count,
                g_InvRing_Source[RT_MAIN].current);
        } else {
            InvRing_InitRing(
                ring, RT_OPTION, g_InvRing_Source[RT_OPTION].items,
                g_InvRing_Source[RT_OPTION].count,
                g_InvRing_Source[RT_OPTION].current);
        }
        break;
    }

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
    return InvRing_Control(&m_Ring, num_frames);
}

static void M_End(void)
{
    INVENTORY_ITEM *const inv_item = m_Ring.list[m_Ring.current_object];
    if (inv_item != NULL) {
        Option_Shutdown(inv_item);
    }

    InvRing_Destroy();
    if (g_Config.input.enable_buffering) {
        g_OldInputDB = (INPUT_STATE) { 0 };
    }
    if (g_InvMode == INV_TITLE_MODE) {
        Music_Stop();
        Sound_StopAllSamples();
    }
}

static void M_Draw(void)
{
    InvRing_Draw(&m_Ring);
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
