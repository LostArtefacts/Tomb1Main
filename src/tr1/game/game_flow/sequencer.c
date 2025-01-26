#include "game/game_flow/sequencer.h"

#include "game/demo.h"
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
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/enum_map.h>
#include <libtrx/game/phase.h>
#include <libtrx/log.h>

static GAME_FLOW_COMMAND M_StorySoFar(
    const GAME_FLOW_LEVEL *level, int32_t savegame_level);

GAME_FLOW_COMMAND
GF_InterpretSequence(const int32_t level_num, GAME_FLOW_LEVEL_TYPE type)
{
    LOG_DEBUG("running sequence for level=%d type=%d", level_num, type);

    g_GameInfo.remove_guns = false;
    g_GameInfo.remove_scions = false;
    g_GameInfo.remove_ammo = false;
    g_GameInfo.remove_medipacks = false;

    GAME_FLOW_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };

    const GAME_FLOW_LEVEL *const level = GF_GetLevel(level_num, type);
    const GAME_FLOW_SEQUENCE *const sequence = &level->sequence;
    for (int32_t i = 0; i < sequence->length; i++) {
        const GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_DEBUG(
            "event type=%s(%d) data=0x%x",
            ENUM_MAP_TO_STRING(GAME_FLOW_SEQUENCE_EVENT_TYPE, event->type),
            event->type, event->data);

        if (!g_Config.gameplay.enable_cine && level->type == GFL_CUTSCENE) {
            bool skip;
            switch (event->type) {
            case GFS_EXIT_TO_TITLE:
            case GFS_EXIT_TO_LEVEL:
            case GFS_PLAY_FMV:
            case GFS_LEVEL_STATS:
            case GFS_TOTAL_STATS:
                skip = false;
                break;
            default:
                skip = true;
                break;
            }
            if (skip) {
                continue;
            }
        }

        switch (event->type) {
        case GFS_LOAD_LEVEL: {
            const int32_t num = (int32_t)(intptr_t)event->data;
            if (type == GFL_DEMO) {
                break;
            }
            if (level->type == GFL_CUTSCENE) {
                gf_cmd = GF_LoadLevel(level_num, GFL_CUTSCENE);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            } else {
                const bool result = Game_Start_Legacy(num, type);
                if (!result) {
                    g_CurrentLevel = -1;
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
            }
            break;
        }

        case GFS_PLAY_LEVEL:
            if (level->type == GFL_CUTSCENE) {
                if (type != GFL_SAVED) {
                    gf_cmd = GF_RunCutscene((int32_t)(intptr_t)event->data);
                    if (gf_cmd.action != GF_NOOP
                        && gf_cmd.action != GF_LEVEL_COMPLETE) {
                        return gf_cmd;
                    }
                }
            } else if (type == GFL_DEMO) {
                return GF_RunDemo(level_num);
            } else {
                if (type != GFL_SAVED
                    && level_num != g_GameFlow.first_level_num) {
                    Lara_RevertToPistolsIfNeeded();
                }
                gf_cmd = GF_RunGame(level_num, type);
                if (type == GFL_SAVED) {
                    type = GFL_NORMAL;
                }
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;

        case GFS_PLAY_FMV: {
            const int16_t fmv_id = (int16_t)(intptr_t)event->data;
            if (type != GFL_SAVED) {
                if (fmv_id < 0 || fmv_id >= g_GameFlow.fmv_count) {
                    LOG_ERROR("Invalid FMV number: %d", fmv_id);
                } else {
                    FMV_Play(g_GameFlow.fmvs[fmv_id].path);
                }
            }
            break;
        }

        case GFS_LEVEL_STATS: {
            PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                .background_type = BK_TRANSPARENT,
                .background_path = NULL,
                .level_num = (int32_t)(intptr_t)event->data,
                .show_final_stats = false,
                .use_bare_style = true,
            });
            gf_cmd = PhaseExecutor_Run(phase);
            Phase_Stats_Destroy(phase);
            if (gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
            break;
        }

        case GFS_TOTAL_STATS:
            if (g_Config.gameplay.enable_total_stats && type != GFL_SAVED) {
                PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                    .background_type = BK_IMAGE,
                    .background_path = event->data,
                    .level_num = level_num,
                    .show_final_stats = true,
                    .use_bare_style = false,
                });
                gf_cmd = PhaseExecutor_Run(phase);
                Phase_Stats_Destroy(phase);
                if (gf_cmd.action != GF_NOOP) {
                    return gf_cmd;
                }
            }
            break;

        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE:
            if (event->type == GFS_LOADING_SCREEN
                && !g_Config.gameplay.enable_loading_screens) {
                break;
            }

            if (type == GFL_SAVED) {
                break;
            }

            if (g_CurrentLevel == -1 && !g_Config.gameplay.enable_eidos_logo) {
                break;
            }

            GAME_FLOW_DISPLAY_PICTURE_DATA *data = event->data;
            PHASE *const phase = Phase_Picture_Create((PHASE_PICTURE_ARGS) {
                .file_name = data->path,
                .display_time = data->display_time,
                .fade_in_time = 1.0,
                .fade_out_time = 1.0,
                .display_time_includes_fades = false,
            });
            gf_cmd = PhaseExecutor_Run(phase);
            Phase_Picture_Destroy(phase);
            if (gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
            break;

        case GFS_EXIT_TO_TITLE:
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };

        case GFS_EXIT_TO_LEVEL: {
            int32_t next_level =
                (int32_t)(intptr_t)event->data & ((1 << 6) - 1);
            if (g_GameFlow.levels[next_level].type == GFL_BONUS
                && !g_GameInfo.bonus_level_unlock) {
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_GAME,
                .param = next_level,
            };
        }

        case GFS_PLAY_CUTSCENE: {
            const int16_t cutscene_num = (int16_t)(intptr_t)event->data;
            if (type != GFL_SAVED) {
                gf_cmd = GF_DoCutsceneSequence(cutscene_num);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_SET_CAMERA_ANGLE:
            g_CinePosition.rot = (int32_t)(intptr_t)event->data;
            break;
        case GFS_FLIP_MAP:
            Room_FlipMap();
            break;
        case GFS_PLAY_MUSIC:
            Music_Play((int32_t)(intptr_t)event->data);
            break;

        case GFS_ADD_ITEM:
            if (type != GFL_SAVED) {
                const GAME_FLOW_ADD_ITEM_DATA *add_item_data =
                    (const GAME_FLOW_ADD_ITEM_DATA *)event->data;
                Inv_AddItemNTimes(
                    add_item_data->object_id, add_item_data->quantity);
            }
            break;

        case GFS_REMOVE_WEAPONS:
            if (type != GFL_SAVED && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
                g_GameInfo.remove_guns = true;
            }
            break;

        case GFS_REMOVE_SCIONS:
            if (type != GFL_SAVED) {
                g_GameInfo.remove_scions = true;
            }
            break;

        case GFS_REMOVE_AMMO:
            if (type != GFL_SAVED && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
                g_GameInfo.remove_ammo = true;
            }
            break;

        case GFS_REMOVE_MEDIPACKS:
            if (type != GFL_SAVED) {
                g_GameInfo.remove_medipacks = true;
            }
            break;

        case GFS_MESH_SWAP: {
            const GAME_FLOW_MESH_SWAP_DATA *const swap_data = event->data;
            Object_SwapMesh(
                swap_data->object1_id, swap_data->object2_id,
                swap_data->mesh_num);
            break;
        }

        case GFS_SETUP_BACON_LARA: {
            int32_t anchor_room = (int32_t)(intptr_t)event->data;
            if (!BaconLara_InitialiseAnchor(anchor_room)) {
                LOG_ERROR(
                    "Could not anchor Bacon Lara to room %d", anchor_room);
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            break;
        }
        }
    }

    return gf_cmd;
}

static GAME_FLOW_COMMAND M_StorySoFar(
    const GAME_FLOW_LEVEL *const level, const int32_t savegame_level)
{
    GAME_FLOW_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };

    const GAME_FLOW_SEQUENCE *const sequence = &level->sequence;
    for (int32_t i = 0; i < sequence->length; i++) {
        const GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_INFO("event %d %d", event->type, event->data);

        switch (event->type) {
        case GFS_LEVEL_STATS:
        case GFS_TOTAL_STATS:
        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE:
        case GFS_ADD_ITEM:
        case GFS_REMOVE_WEAPONS:
        case GFS_REMOVE_SCIONS:
        case GFS_REMOVE_AMMO:
        case GFS_REMOVE_MEDIPACKS:
        case GFS_SETUP_BACON_LARA:
            break;

        case GFS_LOAD_LEVEL: {
            const int32_t level_num = (int32_t)(intptr_t)event->data;
            if (level_num == savegame_level) {
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            } else if (level->type == GFL_CUTSCENE) {
                gf_cmd = GF_LoadLevel(level_num, GFL_CUTSCENE);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_PLAY_LEVEL: {
            const int32_t level_num = (int32_t)(intptr_t)event->data;
            if (level->type == GFL_CUTSCENE) {
                gf_cmd = GF_RunCutscene((int32_t)(intptr_t)event->data);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_PLAY_FMV: {
            const int16_t fmv_id = (int16_t)(intptr_t)event->data;
            if (fmv_id < 0 || fmv_id >= g_GameFlow.fmv_count) {
                LOG_ERROR("Invalid FMV number: %d", fmv_id);
            } else {
                FMV_Play(g_GameFlow.fmvs[fmv_id].path);
            }
            break;
        }

        case GFS_EXIT_TO_TITLE:
            Music_Stop();
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };

        case GFS_EXIT_TO_LEVEL:
            Music_Stop();
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_GAME,
                .param = (int32_t)(intptr_t)event->data & ((1 << 6) - 1),
            };

        case GFS_PLAY_CUTSCENE:
            Music_Stop();
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_CINE,
                .param = (int32_t)(intptr_t)event->data & ((1 << 6) - 1),
            };

        case GFS_SET_CAMERA_ANGLE:
            g_CinePosition.rot = (int32_t)(intptr_t)event->data;
            break;
        case GFS_FLIP_MAP:
            Room_FlipMap();
            break;
        case GFS_PLAY_MUSIC:
            Music_Play((int32_t)(intptr_t)event->data);
            break;

        case GFS_MESH_SWAP: {
            const GAME_FLOW_MESH_SWAP_DATA *const swap_data = event->data;
            Object_SwapMesh(
                swap_data->object1_id, swap_data->object2_id,
                swap_data->mesh_num);
            break;
        }
        }
    }

    return gf_cmd;
}

GAME_FLOW_COMMAND GF_PlayAvailableStory(int32_t slot_num)
{
    GAME_FLOW_COMMAND gf_cmd = {
        .action = GF_START_GAME,
        .param = g_GameFlow.first_level_num,
    };

    const int32_t savegame_level = Savegame_GetLevelNumber(slot_num);
    while (1) {
        gf_cmd =
            M_StorySoFar(GF_GetLevel(gf_cmd.param, GFL_NORMAL), savegame_level);
        if (gf_cmd.action == GF_EXIT_TO_TITLE
            || gf_cmd.action == GF_EXIT_GAME) {
            break;
        }
    }

    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
}

GAME_FLOW_COMMAND GF_LoadLevel(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE type)
{
    if (!Level_Initialise(GF_GetLevel(level_num, type))) {
        return (GAME_FLOW_COMMAND) { .action = type == GFL_TITLE
                                         ? GF_EXIT_GAME
                                         : GF_EXIT_TO_TITLE };
    }
    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

GAME_FLOW_COMMAND GF_DoCutsceneSequence(const int32_t cutscene_num)
{
    const GAME_FLOW_LEVEL *const level =
        GF_GetLevel(cutscene_num, GFL_CUTSCENE);
    if (level == NULL) {
        LOG_ERROR("Missing cutscene: %d", cutscene_num);
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(cutscene_num, GFL_CUTSCENE);
}

GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(demo_num, GFL_DEMO);
}
