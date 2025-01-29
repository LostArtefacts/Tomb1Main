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
    if (seq_ctx == GFSC_STORY) {
        const int32_t savegame_level_num = (int32_t)(intptr_t)seq_ctx_arg;
        if (savegame_level_num == level->num) {
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
    } else if (level->type == GFL_DEMO) {
    } else if (level->type == GFL_CUTSCENE) {
        if (!Level_Initialise(level)) {
            if (level->type == GFL_TITLE) {
                gf_cmd = (GF_COMMAND) { .action = GF_EXIT_GAME };
            } else {
                gf_cmd = (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
        }
    } else {
        const bool result = Game_Start_Legacy(level, seq_ctx);
        if (!result) {
            Game_SetCurrentLevel(NULL);
            return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
    }
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
    if (Game_GetCurrentLevel() == NULL
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
    if (seq_ctx != GFSC_NORMAL) {
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    const GF_LEVEL *const current_level = Game_GetCurrentLevel();
    const GF_LEVEL *const next_level = GF_GetLevelAfter(current_level);
    if (next_level == NULL) {
        return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
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
