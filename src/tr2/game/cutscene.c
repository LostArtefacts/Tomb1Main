#include "game/cutscene.h"

#include "decomp/decomp.h"
#include "game/camera.h"
#include "game/effects.h"
#include "game/game_flow.h"
#include "game/items.h"
#include "game/lara/hair.h"
#include "game/level.h"
#include "game/music.h"
#include "game/output.h"
#include "game/room.h"
#include "game/room_draw.h"
#include "game/shell.h"
#include "game/sound.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/log.h>
#include <libtrx/utils.h>

static void M_FixAudioDrift(void);

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

bool Cutscene_Start(const int32_t level_num)
{
    if (!Level_Initialise(level_num, GFL_CUTSCENE)) {
        return false;
    }

    Room_InitCinematic();
    CutscenePlayer1_Initialise(g_Lara.item_num);
    g_Camera.target_angle = g_CineTargetAngle;

    if (!Music_PlaySynced(g_CineTrackID)) {
        return false;
    }

    Music_SetVolume(10);
    g_CineFrameIdx = 0;
    return true;
}

void Cutscene_End(void)
{
    Music_SetVolume(g_Config.audio.music_volume);
    Music_Stop();
    Sound_StopAll();
}

GAME_FLOW_COMMAND Cutscene_Control(void)
{
    M_FixAudioDrift();

    Input_Update();
    Shell_ProcessInput();
    if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
        return (GAME_FLOW_COMMAND) { .action = GF_LEVEL_COMPLETE };
    } else if (g_InputDB.pause) {
        const GAME_FLOW_COMMAND gf_cmd = GF_PauseGame();
        if (gf_cmd.action != GF_NOOP) {
            return gf_cmd;
        }
    } else if (g_InputDB.toggle_photo_mode) {
        const GAME_FLOW_COMMAND gf_cmd = GF_EnterPhotoMode();
        if (gf_cmd.action != GF_NOOP) {
            return gf_cmd;
        }
    }

    Output_ResetDynamicLights();

    Item_Control();
    Effect_Control();
    Lara_Hair_Control(true);
    Camera_UpdateCutscene();
    Output_AnimateTextures(1);

    g_CineFrameIdx++;
    if (g_CineFrameIdx >= g_NumCineFrames) {
        return (GAME_FLOW_COMMAND) { .action = GF_LEVEL_COMPLETE };
    }

    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

void Cutscene_Draw(void)
{
    g_CameraUnderwater = false;
    Camera_Apply();
    Room_DrawAllRooms(g_Camera.pos.room_num);
    Output_DrawPolyList();
}
