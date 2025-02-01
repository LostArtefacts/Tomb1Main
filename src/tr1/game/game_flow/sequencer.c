#include "game/game_flow/sequencer.h"

#include "game/fmv.h"
#include "game/game.h"
#include "game/game_flow/common.h"
#include "game/game_flow/vars.h"
#include "game/inventory.h"
#include "game/lara/common.h"
#include "game/level.h"
#include "game/music.h"
#include "game/objects/creatures/bacon_lara.h"
#include "game/savegame.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/phase.h>
#include <libtrx/log.h>

#define DECLARE_EVENT_HANDLER(name)                                            \
    GF_COMMAND name(                                                           \
        const GF_LEVEL *const level, const GF_SEQUENCE_EVENT *const event,     \
        const GF_SEQUENCE_CONTEXT seq_ctx, void *const seq_ctx_arg)

static DECLARE_EVENT_HANDLER(M_HandleExitToTitle);
static DECLARE_EVENT_HANDLER(M_HandleLoadLevel);
static DECLARE_EVENT_HANDLER(M_HandlePlayLevel);
static DECLARE_EVENT_HANDLER(M_HandleLevelStats);
static DECLARE_EVENT_HANDLER(M_HandleTotalStats);
static DECLARE_EVENT_HANDLER(M_HandlePicture);
static DECLARE_EVENT_HANDLER(M_HandleLevelComplete);
static DECLARE_EVENT_HANDLER(M_HandlePlayCutscene);
static DECLARE_EVENT_HANDLER(M_HandlePlayFMV);
static DECLARE_EVENT_HANDLER(M_HandlePlayMusic);
static DECLARE_EVENT_HANDLER(M_HandleSetCameraAngle);
static DECLARE_EVENT_HANDLER(M_HandleFlipMap);
static DECLARE_EVENT_HANDLER(M_HandleAddItem);
static DECLARE_EVENT_HANDLER(M_HandleRemoveWeapons);
static DECLARE_EVENT_HANDLER(M_HandleRemoveScions);
static DECLARE_EVENT_HANDLER(M_HandleRemoveAmmo);
static DECLARE_EVENT_HANDLER(M_HandleRemoveMedipacks);
static DECLARE_EVENT_HANDLER(M_HandleMeshSwap);
static DECLARE_EVENT_HANDLER(M_HandleSetupBaconLara);

static DECLARE_EVENT_HANDLER((*m_EventHandlers[GFS_NUMBER_OF])) = {
    // clang-format off
    [GFS_EXIT_TO_TITLE]    = M_HandleExitToTitle,
    [GFS_LOAD_LEVEL]       = M_HandleLoadLevel,
    [GFS_PLAY_LEVEL]       = M_HandlePlayLevel,
    [GFS_LEVEL_STATS]      = M_HandleLevelStats,
    [GFS_TOTAL_STATS]      = M_HandleTotalStats,
    [GFS_LOADING_SCREEN]   = M_HandlePicture,
    [GFS_DISPLAY_PICTURE]  = M_HandlePicture,
    [GFS_LEVEL_COMPLETE]   = M_HandleLevelComplete,
    [GFS_PLAY_CUTSCENE]    = M_HandlePlayCutscene,
    [GFS_PLAY_FMV]         = M_HandlePlayFMV,
    [GFS_PLAY_MUSIC]       = M_HandlePlayMusic,
    [GFS_SET_CAMERA_ANGLE] = M_HandleSetCameraAngle,
    [GFS_FLIP_MAP]         = M_HandleFlipMap,
    [GFS_ADD_ITEM]         = M_HandleAddItem,
    [GFS_REMOVE_WEAPONS]   = M_HandleRemoveWeapons,
    [GFS_REMOVE_SCIONS]    = M_HandleRemoveScions,
    [GFS_REMOVE_AMMO]      = M_HandleRemoveAmmo,
    [GFS_REMOVE_MEDIPACKS] = M_HandleRemoveMedipacks,
    [GFS_MESH_SWAP]        = M_HandleMeshSwap,
    [GFS_SETUP_BACON_LARA] = M_HandleSetupBaconLara,
    // clang-format on
};

static DECLARE_EVENT_HANDLER(M_HandleExitToTitle)
{
    return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
}

static DECLARE_EVENT_HANDLER(M_HandleLoadLevel)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    const GF_LEVEL *const prev_level = GF_GetLevelBefore(level);

    switch (seq_ctx) {
    case GFSC_STORY:
        const int32_t savegame_level_num = (int32_t)(intptr_t)seq_ctx_arg;
        if (savegame_level_num == level->num) {
            gf_cmd = (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        break;

    case GFSC_SAVED:
        // reset current info to the defaults so that we do not do
        // Item_GlobalReplace in the inventory initialization routines too early
        Savegame_InitCurrentInfo();
        const int16_t slot_num = Savegame_GetBoundSlot();

        if (!Level_Initialise(level)) {
            Game_SetCurrentLevel(nullptr);
            GF_SetCurrentLevel(nullptr);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        if (!Savegame_Load(slot_num)) {
            LOG_ERROR("Failed to load save file!");
            Game_SetCurrentLevel(nullptr);
            GF_SetCurrentLevel(nullptr);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        break;

    case GFSC_RESTART:
        if (level == GF_GetGymLevel() || level == GF_GetFirstLevel()) {
            Savegame_InitCurrentInfo();
        } else {
            Savegame_ResetCurrentInfo(level);
            Savegame_CarryCurrentInfoToNextLevel(prev_level, level);
            Savegame_ApplyLogicToCurrentInfo(level);
        }
        if (!Level_Initialise(level)) {
            Game_SetCurrentLevel(nullptr);
            GF_SetCurrentLevel(nullptr);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        break;

    case GFSC_SELECT:
        if (Savegame_GetBoundSlot() != -1) {
            // select level feature
            Savegame_InitCurrentInfo();
            if (level->num > GF_GetFirstLevel()->num) {
                Savegame_LoadOnlyResumeInfo(
                    Savegame_GetBoundSlot(), &g_GameInfo);
                const GF_LEVEL *tmp_level = level;
                while (tmp_level != nullptr) {
                    Savegame_ResetCurrentInfo(tmp_level);
                    tmp_level = GF_GetLevelAfter(tmp_level);
                }
                Savegame_CarryCurrentInfoToNextLevel(prev_level, level);
                Savegame_ApplyLogicToCurrentInfo(level);
            }
        } else {
            // console /play level feature
            Savegame_InitCurrentInfo();
            const GF_LEVEL *tmp_level = GF_GetLevelAfter(GF_GetFirstLevel());
            while (tmp_level != nullptr) {
                Savegame_CarryCurrentInfoToNextLevel(
                    GF_GetLevelBefore(tmp_level), tmp_level);
                Savegame_ApplyLogicToCurrentInfo(tmp_level);
                if (tmp_level == level) {
                    break;
                }
                tmp_level = GF_GetLevelAfter(tmp_level);
            }
        }
        if (!Level_Initialise(level)) {
            Game_SetCurrentLevel(nullptr);
            GF_SetCurrentLevel(nullptr);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        break;

    default:
        if (level->type == GFL_GYM) {
            Savegame_ResetCurrentInfo(level);
            Savegame_ApplyLogicToCurrentInfo(level);
        } else if (level->type == GFL_BONUS) {
            Savegame_CarryCurrentInfoToNextLevel(prev_level, level);
            Savegame_ApplyLogicToCurrentInfo(level);
        }

        if (!Level_Initialise(level)) {
            Game_SetCurrentLevel(nullptr);
            GF_SetCurrentLevel(nullptr);
            if (level->type == GFL_TITLE) {
                gf_cmd = (GF_COMMAND) { .action = GF_EXIT_GAME };
            } else {
                gf_cmd = (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
        }
        break;
    }

    Stats_CalculateStats();
    RESUME_INFO *const resume = GF_GetResumeInfo(level);
    if (resume != nullptr) {
        resume->stats.max_pickup_count = Stats_GetPickups();
        resume->stats.max_kill_count = Stats_GetKillables();
        resume->stats.max_secret_count = Stats_GetSecrets();
    }

    g_GameInfo.ask_for_save = g_Config.gameplay.enable_save_crystals
        && (level->type == GFL_NORMAL || level->type == GFL_BONUS)
        && level != GF_GetFirstLevel() && level != GF_GetGymLevel();

    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandlePlayLevel)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (level->type == GFL_CUTSCENE) {
        if (seq_ctx != GFSC_SAVED) {
            gf_cmd = GF_RunCutscene((int32_t)(intptr_t)event->data);
            if (gf_cmd.action == GF_LEVEL_COMPLETE) {
                gf_cmd.action = GF_NOOP;
            }
        }
    } else if (seq_ctx == GFSC_STORY) {
        const int32_t savegame_level_num = (int32_t)(intptr_t)seq_ctx_arg;
        if (savegame_level_num == level->num) {
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
    } else if (level->type == GFL_DEMO) {
        return GF_RunDemo(level->num);
    } else {
        if (seq_ctx != GFSC_SAVED && level != GF_GetFirstLevel()) {
            Lara_RevertToPistolsIfNeeded();
        }
        gf_cmd = GF_RunGame(level, seq_ctx);
        if (gf_cmd.action == GF_LEVEL_COMPLETE) {
            gf_cmd.action = GF_NOOP;
        }
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleLevelStats)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx == GFSC_NORMAL) {
        const GF_LEVEL *const current_level = Game_GetCurrentLevel();
        PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
            .background_type = BK_TRANSPARENT,
            .level_num = current_level->num,
            .show_final_stats = false,
            .use_bare_style = true,
        });
        gf_cmd = PhaseExecutor_Run(phase);
        Phase_Stats_Destroy(phase);
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleTotalStats)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx == GFSC_NORMAL && g_Config.gameplay.enable_total_stats) {
        PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
            .background_type = BK_IMAGE,
            .background_path = event->data,
            .level_num = level->num,
            .show_final_stats = true,
            .use_bare_style = false,
        });
        gf_cmd = PhaseExecutor_Run(phase);
        Phase_Stats_Destroy(phase);
    }
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandlePicture)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (event->type == GFS_LOADING_SCREEN
        && !g_Config.gameplay.enable_loading_screens) {
        return gf_cmd;
    }
    if (seq_ctx == GFSC_SAVED) {
        return gf_cmd;
    }
    if (Game_GetCurrentLevel() == nullptr
        && !g_Config.gameplay.enable_eidos_logo) {
        return gf_cmd;
    }

    GF_DISPLAY_PICTURE_DATA *data = event->data;
    PHASE *const phase = Phase_Picture_Create((PHASE_PICTURE_ARGS) {
        .file_name = data->path,
        .display_time = data->display_time,
        .fade_in_time = data->fade_in_time,
        .fade_out_time = data->fade_out_time,
        .display_time_includes_fades = false,
    });
    gf_cmd = PhaseExecutor_Run(phase);
    Phase_Picture_Destroy(phase);
    return gf_cmd;
}

static DECLARE_EVENT_HANDLER(M_HandleLevelComplete)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    if (seq_ctx != GFSC_NORMAL) {
        return gf_cmd;
    }
    const GF_LEVEL *const current_level = Game_GetCurrentLevel();
    const GF_LEVEL *const next_level = GF_GetLevelAfter(current_level);

    if (current_level == GF_GetLastLevel()) {
        g_Config.profile.new_game_plus_unlock = true;
        Config_Write();
        g_GameInfo.bonus_level_unlock =
            Stats_CheckAllSecretsCollected(GFL_NORMAL);
    }

    // play specific level
    if (g_GameInfo.select_level_num != -1) {
        const GF_LEVEL *const select_level =
            GF_GetLevel(GFLT_MAIN, g_GameInfo.select_level_num);
        if (current_level != nullptr && select_level != nullptr) {
            Savegame_CarryCurrentInfoToNextLevel(current_level, select_level);
        }
        return (GF_COMMAND) {
            .action = GF_SELECT_GAME,
            .param = g_GameInfo.select_level_num,
        };
    }

    // missing level
    if (next_level == nullptr) {
        return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }

    // carry info to the next level
    Savegame_CarryCurrentInfoToNextLevel(current_level, next_level);
    Savegame_ApplyLogicToCurrentInfo(next_level);

    if (next_level->type == GFL_BONUS && !g_GameInfo.bonus_level_unlock) {
        return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    return (GF_COMMAND) {
        .action = GF_START_GAME,
        .param = next_level->num,
    };
}

static DECLARE_EVENT_HANDLER(M_HandlePlayCutscene)
{
    GF_COMMAND gf_cmd = { .action = GF_NOOP };
    Music_Stop();
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
    const int16_t fmv_id = (int16_t)(intptr_t)event->data;
    if (seq_ctx != GFSC_SAVED) {
        if (fmv_id < 0 || fmv_id >= g_GameFlow.fmv_count) {
            LOG_ERROR("Invalid FMV number: %d", fmv_id);
        } else {
            FMV_Play(g_GameFlow.fmvs[fmv_id].path);
        }
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandlePlayMusic)
{
    Music_Play((int32_t)(intptr_t)event->data);
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleSetCameraAngle)
{
    if (seq_ctx != GFSC_STORY) {
        g_CinePosition.rot = (int32_t)(intptr_t)event->data;
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleFlipMap)
{
    if (seq_ctx != GFSC_STORY) {
        Room_FlipMap();
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleAddItem)
{
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
        const GF_ADD_ITEM_DATA *add_item_data =
            (const GF_ADD_ITEM_DATA *)event->data;
        Inv_AddItemNTimes(add_item_data->object_id, add_item_data->quantity);
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleRemoveWeapons)
{
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED
        && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
        g_GameInfo.remove_guns = true;
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleRemoveScions)
{
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
        g_GameInfo.remove_scions = true;
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleRemoveAmmo)
{
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED
        && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
        g_GameInfo.remove_ammo = true;
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleRemoveMedipacks)
{
    if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
        g_GameInfo.remove_medipacks = true;
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleMeshSwap)
{
    if (seq_ctx != GFSC_STORY) {
        const GF_MESH_SWAP_DATA *const swap_data = event->data;
        Object_SwapMesh(
            swap_data->object1_id, swap_data->object2_id, swap_data->mesh_num);
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

static DECLARE_EVENT_HANDLER(M_HandleSetupBaconLara)
{
    if (seq_ctx != GFSC_STORY) {
        const int32_t anchor_room = (int32_t)(intptr_t)event->data;
        if (!BaconLara_InitialiseAnchor(anchor_room)) {
            LOG_ERROR("Could not anchor Bacon Lara to room %d", anchor_room);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
    }
    return (GF_COMMAND) { .action = GF_NOOP };
}

void GF_PreSequenceHook(void)
{
    g_GameInfo.remove_guns = false;
    g_GameInfo.remove_scions = false;
    g_GameInfo.remove_ammo = false;
    g_GameInfo.remove_medipacks = false;
}

GF_SEQUENCE_CONTEXT GF_SwitchSequenceContext(
    const GF_SEQUENCE_EVENT *const event, const GF_SEQUENCE_CONTEXT seq_ctx)
{
    if (event->type != GFS_LOAD_LEVEL && event->type != GFS_PLAY_LEVEL) {
        return seq_ctx;
    }
    switch (seq_ctx) {
    case GFSC_SAVED:
    case GFSC_RESTART:
    case GFSC_SELECT:
        return GFSC_NORMAL;
    default:
        return seq_ctx;
    }
}

bool GF_ShouldSkipSequenceEvent(
    const GF_LEVEL *const level, const GF_SEQUENCE_EVENT *const event)
{
    // Skip cinematic levels
    if (!g_Config.gameplay.enable_cine && level->type == GFL_CUTSCENE) {
        switch (event->type) {
        case GFS_EXIT_TO_TITLE:
        case GFS_LEVEL_COMPLETE:
        case GFS_PLAY_FMV:
        case GFS_LEVEL_STATS:
        case GFS_TOTAL_STATS:
            return false;
        default:
            return true;
        }
    }
    return false;
}

GF_COMMAND(*GF_GetSequenceEventHandler(GF_SEQUENCE_EVENT_TYPE event_type))
(const GF_LEVEL *, const GF_SEQUENCE_EVENT *, GF_SEQUENCE_CONTEXT, void *)
{
    return m_EventHandlers[event_type];
}
