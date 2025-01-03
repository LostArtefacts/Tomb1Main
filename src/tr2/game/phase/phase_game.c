#include "game/phase/phase_game.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/camera.h"
#include "game/game.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/phase/priv.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef struct {
    bool exiting;
    FADER exit_fader;
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

    if (p->level_type == GFL_NORMAL || p->level_type == GFL_SAVED
        || p->level_type == GFL_DEMO) {
        g_CurrentLevel = p->level_num;
    }
    if (p->level_type != GFL_SAVED) {
        ModifyStartInfo(p->level_num);
    }
    if (p->level_type != GFL_SAVED) {
        InitialiseLevelFlags();
    }
    if (!Level_Initialise(p->level_num, p->level_type)) {
        g_CurrentLevel = 0;
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_GAME },
        };
    }

    g_OverlayStatus = 1;
    Camera_Initialise();
    Stats_StartTimer();

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Overlay_HideGameInfo();
    Sound_StopAllSamples();
    Music_Stop();
    Music_SetVolume(g_Config.audio.music_volume);
}

static void M_Resume(PHASE *const phase)
{
    Stats_StartTimer();
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    GAME_FLOW_COMMAND gf_cmd;
    if (g_IsGameToExit && !p->exiting) {
        p->exiting = true;
        Fader_InitAnyToBlack(&p->exit_fader, FRAMES_PER_SECOND / 3);
    } else if (p->exiting && !Fader_IsActive(&p->exit_fader)) {
        gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
    } else {
        Fader_Control(&p->exit_fader);
        gf_cmd = Game_Control(num_frames, false);
    }

    if (gf_cmd.action != GF_NOOP) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = gf_cmd,
        };
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Game_Draw();
    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->exit_fader));
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
