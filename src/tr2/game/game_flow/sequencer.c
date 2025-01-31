#include "game/game_flow/sequencer.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/fmv.h"
#include "game/game.h"
#include "game/game_flow.h"
#include "game/level.h"
#include "game/music.h"
#include "game/phase.h"
#include "global/vars.h"

#include <libtrx/log.h>

#define DECLARE_EVENT_HANDLER(name)                                            \
    GF_COMMAND name(                                                           \
        const GF_LEVEL *const level, const GF_SEQUENCE_EVENT *const event,     \
        GF_SEQUENCE_CONTEXT seq_ctx, void *const seq_ctx_arg)

static DECLARE_EVENT_HANDLER(M_HandleExitToTitle);
static DECLARE_EVENT_HANDLER(M_HandlePlayLevel);
static DECLARE_EVENT_HANDLER(M_HandlePlayCutscene);
static DECLARE_EVENT_HANDLER(M_HandlePlayMusic);
static DECLARE_EVENT_HANDLER(M_HandlePlayFMV);
static DECLARE_EVENT_HANDLER(M_HandlePicture);
static DECLARE_EVENT_HANDLER(M_HandleLevelComplete);
static DECLARE_EVENT_HANDLER(M_HandleLevelStats);
static DECLARE_EVENT_HANDLER(M_HandleEnableSunset);
static DECLARE_EVENT_HANDLER(M_HandleTotalStats);
static DECLARE_EVENT_HANDLER(M_HandleSetCameraAngle);
static DECLARE_EVENT_HANDLER(M_HandleDisableFloor);
static DECLARE_EVENT_HANDLER(M_HandleAddItem);
static DECLARE_EVENT_HANDLER(M_HandleAddSecretReward);
static DECLARE_EVENT_HANDLER(M_HandleRemoveWeapons);
static DECLARE_EVENT_HANDLER(M_HandleRemoveAmmo);
static DECLARE_EVENT_HANDLER(M_HandleSetStartAnim);
static DECLARE_EVENT_HANDLER(M_HandleSetNumSecrets);

static DECLARE_EVENT_HANDLER((*m_EventHandlers[GFS_NUMBER_OF])) = {
    // clang-format off
    [GFS_EXIT_TO_TITLE]     = M_HandleExitToTitle,
    [GFS_PLAY_LEVEL]        = M_HandlePlayLevel,
    [GFS_PLAY_CUTSCENE]     = M_HandlePlayCutscene,
    [GFS_PLAY_MUSIC]        = M_HandlePlayMusic,
    [GFS_PLAY_FMV]          = M_HandlePlayFMV,
    [GFS_DISPLAY_PICTURE]   = M_HandlePicture,
    [GFS_LEVEL_COMPLETE]    = M_HandleLevelComplete,
    [GFS_LEVEL_STATS]       = M_HandleLevelStats,
    [GFS_TOTAL_STATS]       = M_HandleTotalStats,
    [GFS_ENABLE_SUNSET]     = M_HandleEnableSunset,
    [GFS_SET_CAMERA_ANGLE]  = M_HandleSetCameraAngle,
    [GFS_DISABLE_FLOOR]     = M_HandleDisableFloor,
    [GFS_ADD_ITEM]          = M_HandleAddItem,
    [GFS_ADD_SECRET_REWARD] = M_HandleAddSecretReward,
    [GFS_REMOVE_WEAPONS]    = M_HandleRemoveWeapons,
    [GFS_REMOVE_AMMO]       = M_HandleRemoveAmmo,
    [GFS_SET_START_ANIM]    = M_HandleSetStartAnim,
    [GFS_SET_NUM_SECRETS]   = M_HandleSetNumSecrets,
    // clang-format on
};

static DECLARE_EVENT_HANDLER(M_HandleExitToTitle)
{
    return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
}

static DECLARE_EVENT_HANDLER(M_HandlePlayLevel)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx == GFSC_STORY) {
        return gf_cmd;
    } else if (level->type == GFL_DEMO) {
        if (!Level_Initialise(level, seq_ctx)) {
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        gf_cmd = GF_RunDemo(level->num);
    } else if (level->type == GFL_CUTSCENE) {
        if (!Level_Initialise(level, seq_ctx)) {
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        gf_cmd = GF_RunCutscene(level->num);
    } else {
        if (seq_ctx != GFSC_SAVED) {
            if (level != nullptr) {
                Savegame_ApplyLogicToCurrentInfo(level);
            }
            InitialiseLevelFlags();
        }
        if (!Level_Initialise(level, seq_ctx)) {
            Game_SetCurrentLevel(nullptr);
            GF_SetCurrentLevel(nullptr);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        gf_cmd = GF_RunGame(level, seq_ctx);
    }
    if (gf_cmd.action == GF_LEVEL_COMPLETE) {
        gf_cmd.action = GF_NOOP;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandlePlayCutscene)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    const int16_t cutscene_num = (int16_t)(intptr_t)event->data;
    if (seq_ctx != GFSC_SAVED) {
        gf_cmd = GF_DoCutsceneSequence(cutscene_num);
        if (gf_cmd.action == GF_LEVEL_COMPLETE) {
            gf_cmd.action = GF_NOOP;
        }
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandlePlayFMV)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    const int16_t fmv_id = (int16_t)(intptr_t)event->data;
    if (seq_ctx != GFSC_SAVED) {
        if (fmv_id < 0 || fmv_id >= g_GameFlow.fmv_count) {
            LOG_ERROR("Invalid FMV number: %d", fmv_id);
        } else {
            FMV_Play(g_GameFlow.fmvs[fmv_id].path);
        }
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandlePlayMusic)
{
    Music_Play((int32_t)(intptr_t)event->data, MPM_ALWAYS);
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandlePicture)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx == GFSC_SAVED) {
        return gf_cmd;
    }
    const GF_DISPLAY_PICTURE_DATA *const data = event->data;
    PHASE *const phase = Phase_Picture_Create((PHASE_PICTURE_ARGS) {
        .file_name = data->path,
        .display_time = data->display_time,
        .fade_in_time = data->fade_in_time,
        .fade_out_time = data->fade_out_time,
        .display_time_includes_fades = true,
    });
    gf_cmd = PhaseExecutor_Run(phase);
    Phase_Picture_Destroy(phase);
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleLevelComplete)
{
    if (seq_ctx != GFSC_NORMAL) {
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    const GF_LEVEL *const current_level = Game_GetCurrentLevel();
    const GF_LEVEL *const next_level = GF_GetLevelAfter(current_level);
    START_INFO *const start = Savegame_GetCurrentInfo(current_level);
    start->stats = g_SaveGame.current_stats;
    start->available = 0;
    if (next_level != nullptr) {
        Savegame_PersistGameToCurrentInfo(next_level);
        g_SaveGame.current_level = next_level->num;
    }
    if (next_level == nullptr) {
        return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    return (GF_COMMAND) {
        .action = GF_START_GAME,
        .param = next_level->num,
    };
}

static DECLARE_EVENT_HANDLER(M_HandleLevelStats)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx == GFSC_NORMAL) {
        const GF_LEVEL *const current_level = Game_GetCurrentLevel();
        PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
            .background_type = Game_IsInGym() ? BK_TRANSPARENT : BK_OBJECT,
            .level_num = current_level->num,
            .show_final_stats = false,
            .use_bare_style = false,
        });
        gf_cmd = PhaseExecutor_Run(phase);
        Phase_Stats_Destroy(phase);
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleTotalStats)
{
    GF_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };
    if (seq_ctx == GFSC_NORMAL) {
        const GF_LEVEL *const current_level = Game_GetCurrentLevel();
        // TODO: move me out
        START_INFO *const start = Savegame_GetCurrentInfo(current_level);
        start->stats = g_SaveGame.current_stats;
        g_SaveGame.bonus_flag = true;
        PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
            .background_type = BK_IMAGE,
            .background_path = "data/end.pcx",
            .show_final_stats = true,
            .use_bare_style = false,
        });
        gf_cmd = PhaseExecutor_Run(phase);
        Phase_Stats_Destroy(phase);
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleEnableSunset)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY) {
        g_GF_SunsetEnabled = true;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleSetCameraAngle)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_SAVED) {
        g_CineTargetAngle = (int16_t)(intptr_t)event->data;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleDisableFloor)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY) {
        g_GF_NoFloor = (int16_t)(intptr_t)event->data;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleAddItem)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY) {
        const GF_ADD_ITEM_DATA *const data =
            (const GF_ADD_ITEM_DATA *)event->data;
        GF_InventoryModifier_Add(
            data->object_id, data->inv_type, data->quantity);
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleAddSecretReward)
{
    return M_HandleAddItem(level, event, seq_ctx, seq_ctx_arg);
}

static DECLARE_EVENT_HANDLER(M_HandleRemoveWeapons)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
        g_GF_RemoveWeapons = true;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleRemoveAmmo)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
        g_GF_RemoveAmmo = true;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleSetStartAnim)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY) {
        g_GF_LaraStartAnim = (int16_t)(intptr_t)event->data;
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleSetNumSecrets)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_STORY) {
        g_GF_NumSecrets = (int16_t)(intptr_t)event->data;
    }
    return gf_cmd;
}

void GF_PreSequenceHook(void)
{
    g_GF_NoFloor = 0;
    g_GF_SunsetEnabled = false;
    g_GF_LaraStartAnim = 0;
    g_GF_RemoveAmmo = false;
    g_GF_RemoveWeapons = false;
    g_CineTargetAngle = DEG_90;
    g_GF_NumSecrets = 3;
    GF_InventoryModifier_Reset();
}

GF_SEQUENCE_CONTEXT GF_SwitchSequenceContext(
    const GF_SEQUENCE_EVENT *const event, const GF_SEQUENCE_CONTEXT seq_ctx)
{
    // Update sequence context if necessary
    if (event->type == GFS_PLAY_LEVEL && seq_ctx == GFSC_SAVED) {
        return GFSC_NORMAL;
    }
    return seq_ctx;
}

bool GF_ShouldSkipSequenceEvent(
    const GF_LEVEL *const level, const GF_SEQUENCE_EVENT *const event)
{
    return false;
}

GF_COMMAND(*GF_GetSequenceEventHandler(GF_SEQUENCE_EVENT_TYPE event_type))
(const GF_LEVEL *, const GF_SEQUENCE_EVENT *, GF_SEQUENCE_CONTEXT, void *)
{
    return m_EventHandlers[event_type];
}
