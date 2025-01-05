#include "game/phase/phase_inventory.h"

#include "game/input.h"
#include "game/interpolation.h"
#include "game/inventory_ring.h"
#include "game/inventory_ring/priv.h"
#include "game/music.h"
#include "game/option.h"
#include "game/output.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>

RING_INFO m_Ring;
IMOTION_INFO m_Motion;

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

    RING_INFO *ring = &m_Ring;
    IMOTION_INFO *motion = &m_Motion;

    memset(motion, 0, sizeof(IMOTION_INFO));
    memset(ring, 0, sizeof(RING_INFO));

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
            ring, RT_OPTION, g_InvOptionList, g_InvOptionObjects,
            g_InvOptionCurrent, motion);
        break;

    case INV_KEYS_MODE:
        InvRing_InitRing(
            ring, RT_KEYS, g_InvKeysList, g_InvKeysObjects, g_InvMainCurrent,
            motion);
        break;

    default:
        if (g_InvMainObjects) {
            InvRing_InitRing(
                ring, RT_MAIN, g_InvMainList, g_InvMainObjects,
                g_InvMainCurrent, motion);
        } else {
            InvRing_InitRing(
                ring, RT_OPTION, g_InvOptionList, g_InvOptionObjects,
                g_InvOptionCurrent, motion);
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
    return InvRing_Control(&m_Ring, &m_Motion, num_frames);
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
    RING_INFO *ring = &m_Ring;
    IMOTION_INFO *motion = &m_Motion;
    InvRing_Draw(ring, motion);
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
