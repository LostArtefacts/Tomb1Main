#include "game/phase/phase_cutscene.h"

#include "decomp/decomp.h"
#include "game/camera.h"
#include "game/console/common.h"
#include "game/effects.h"
#include "game/game.h"
#include "game/input.h"
#include "game/items.h"
#include "game/lara/hair.h"
#include "game/music.h"
#include "game/output.h"
#include "game/room.h"
#include "game/room_draw.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/fader.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>

typedef struct {
    int32_t level_num;
    bool exiting;
    bool old_sound_active;
    FADER exit_fader;
} M_PRIV;

static void M_FixAudioDrift(void);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static void M_Suspend(PHASE *phase);
static void M_Resume(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static void M_FixAudioDrift(void)
{
    const int32_t audio_frame_idx = Music_GetTimestamp() * FRAMES_PER_SECOND;
    const int32_t game_frame_idx = g_CineFrameIdx;
    const int32_t audio_drift = ABS(audio_frame_idx - game_frame_idx);
    if (audio_drift >= FRAMES_PER_SECOND * 0.2) {
        LOG_DEBUG("Detected audio drift: %d frames", audio_drift);
        Music_SeekTimestamp(game_frame_idx / (double)FRAMES_PER_SECOND);
    }
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    if (!Level_Initialise(p->level_num, GFL_CUTSCENE)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }

    Room_InitCinematic();
    CutscenePlayer1_Initialise(g_Lara.item_num);
    g_Camera.target_angle = g_CineTargetAngle;

    p->old_sound_active = g_SoundIsActive;
    g_SoundIsActive = false;

    if (!Music_PlaySynced(g_CineTrackID)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }

    Music_SetVolume(10);
    Game_SetIsPlaying(true);
    g_CineFrameIdx = 0;
    return (PHASE_CONTROL) {};
}

static void M_End(PHASE *const phase)
{
    Game_SetIsPlaying(false);
    M_PRIV *const p = phase->priv;
    Music_SetVolume(g_Config.audio.music_volume);
    Music_Stop();
    g_SoundIsActive = p->old_sound_active;
    Sound_StopAllSamples();

    g_LevelComplete = true;
}

static void M_Suspend(PHASE *const phase)
{
    Game_SetIsPlaying(false);
}

static void M_Resume(PHASE *const phase)
{
    Game_SetIsPlaying(true);
    Stats_StartTimer();
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    if (g_IsGameToExit && !p->exiting) {
        p->exiting = true;
        Fader_InitAnyToBlack(&p->exit_fader, FRAMES_PER_SECOND / 3);
    } else if (p->exiting && !Fader_IsActive(&p->exit_fader)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_GAME },
        };
    } else {
        M_FixAudioDrift();
        Fader_Control(&p->exit_fader);

        Input_Update();
        if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }
        Shell_ProcessInput();

        g_DynamicLightCount = 0;

        Item_Control();
        Effect_Control();
        Lara_Hair_Control(true);
        Camera_UpdateCutscene();

        g_CineFrameIdx++;
        if (g_CineFrameIdx >= g_NumCineFrames) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }

        g_Camera.num_frames = num_frames;
        Output_AnimateTextures(num_frames);
    }

    return (PHASE_CONTROL) {};
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    g_CameraUnderwater = false;
    Room_DrawAllRooms(g_Camera.pos.room_num);
    Output_DrawPolyList();
    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->exit_fader));
}

PHASE *Phase_Cutscene_Create(const int32_t level_num)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->level_num = level_num;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->suspend = M_Suspend;
    phase->resume = M_Resume;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Cutscene_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
