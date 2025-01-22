#include "game/game_flow/sequencer.h"

#include "game/fmv.h"
#include "game/game.h"
#include "game/game_flow/vars.h"
#include "game/inventory.h"
#include "game/lara/common.h"
#include "game/level.h"
#include "game/music.h"
#include "game/objects/creatures/bacon_lara.h"
#include "game/savegame.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/phase.h>
#include <libtrx/log.h>

GAME_FLOW_COMMAND
GF_InterpretSequence(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type)
{
    LOG_INFO("level_num=%d level_type=%d", level_num, level_type);

    g_GameInfo.remove_guns = false;
    g_GameInfo.remove_scions = false;
    g_GameInfo.remove_ammo = false;
    g_GameInfo.remove_medipacks = false;

    GAME_FLOW_COMMAND command = { .action = GF_EXIT_TO_TITLE };

    const GAME_FLOW_SEQUENCE *const sequence =
        &g_GameFlow.levels[level_num].sequence;
    for (int32_t i = 0; i < sequence->length; i++) {
        const GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_INFO("event %d %d", event->type, event->data);

        if (!g_Config.gameplay.enable_cine
            && g_GameFlow.levels[level_num].level_type == GFL_CUTSCENE) {
            bool skip;
            switch (event->type) {
            case GFS_EXIT_TO_TITLE:
            case GFS_EXIT_TO_LEVEL:
            case GFS_EXIT_TO_CINE:
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
        case GFS_START_GAME: {
            const int32_t num = (int32_t)(intptr_t)event->data;
            if (level_type == GFL_DEMO) {
                break;
            }
            if (g_GameFlow.levels[level_num].level_type == GFL_CUTSCENE) {
                command = GF_LoadLevel(level_num, GFL_CUTSCENE);
                if (command.action != GF_NOOP
                    && command.action != GF_LEVEL_COMPLETE) {
                    return command;
                }
            } else {
                const bool result = Game_Start_Legacy(num, level_type);
                if (!result) {
                    g_CurrentLevel = -1;
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
            }
            break;
        }

        case GFS_LOOP_GAME:
            if (level_type == GFL_DEMO) {
                PHASE *const phase = Phase_Demo_Create(level_num);
                const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
                Phase_Demo_Destroy(phase);
                return gf_cmd;
            } else {
                if (level_type != GFL_SAVED
                    && level_num != g_GameFlow.first_level_num) {
                    Lara_RevertToPistolsIfNeeded();
                }
                command = GF_PlayLevel(level_num, level_type);
                if (command.action != GF_NOOP
                    && command.action != GF_LEVEL_COMPLETE) {
                    return command;
                }
            }
            break;

        case GFS_STOP_GAME:
            command = Game_Stop_Legacy();
            if (command.action != GF_NOOP
                && command.action != GF_LEVEL_COMPLETE) {
                return command;
            }
            if (level_type == GFL_SAVED) {
                if (g_GameFlow.levels[level_num].level_type == GFL_BONUS) {
                    level_type = GFL_BONUS;
                } else {
                    level_type = GFL_NORMAL;
                }
            }
            break;

        case GFS_LOOP_CINE:
            if (level_type != GFL_SAVED) {
                command = GF_PlayCutscene((int32_t)(intptr_t)event->data);
                if (command.action != GF_NOOP
                    && command.action != GF_LEVEL_COMPLETE) {
                    return command;
                }
            }
            break;

        case GFS_PLAY_FMV:
            if (level_type != GFL_SAVED) {
                FMV_Play((char *)event->data);
            }
            break;

        case GFS_LEVEL_STATS: {
            PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                .background_type = BK_TRANSPARENT,
                .background_path = NULL,
                .level_num = (int32_t)(intptr_t)event->data,
                .show_final_stats = false,
                .use_bare_style = true,
            });
            command = PhaseExecutor_Run(phase);
            Phase_Stats_Destroy(phase);
            if (command.action != GF_NOOP) {
                return command;
            }
            break;
        }

        case GFS_TOTAL_STATS:
            if (g_Config.gameplay.enable_total_stats
                && level_type != GFL_SAVED) {
                const GAME_FLOW_DISPLAY_PICTURE_DATA *data = event->data;
                PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                    .background_type = BK_IMAGE,
                    .background_path = data->path,
                    .level_num = level_num,
                    .show_final_stats = true,
                    .use_bare_style = false,
                });
                command = PhaseExecutor_Run(phase);
                Phase_Stats_Destroy(phase);
                if (command.action != GF_NOOP) {
                    return command;
                }
            }
            break;

        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE:
            if (event->type == GFS_LOADING_SCREEN
                && !g_Config.gameplay.enable_loading_screens) {
                break;
            }

            if (level_type == GFL_SAVED) {
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
            command = PhaseExecutor_Run(phase);
            Phase_Picture_Destroy(phase);
            if (command.action != GF_NOOP) {
                return command;
            }
            break;

        case GFS_EXIT_TO_TITLE:
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };

        case GFS_EXIT_TO_LEVEL: {
            int32_t next_level =
                (int32_t)(intptr_t)event->data & ((1 << 6) - 1);
            if (g_GameFlow.levels[next_level].level_type == GFL_BONUS
                && !g_GameInfo.bonus_level_unlock) {
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_GAME,
                .param = next_level,
            };
        }

        case GFS_EXIT_TO_CINE:
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_CINE,
                .param = (int32_t)(intptr_t)event->data & ((1 << 6) - 1),
            };

        case GFS_SET_CAM_ANGLE:
            g_CinePosition.rot = (int32_t)(intptr_t)event->data;
            break;
        case GFS_FLIP_MAP:
            Room_FlipMap();
            break;
        case GFS_PLAY_SYNCED_AUDIO:
            Music_Play((int32_t)(intptr_t)event->data);
            break;

        case GFS_GIVE_ITEM:
            if (level_type != GFL_SAVED) {
                const GAME_FLOW_GIVE_ITEM_DATA *give_item_data =
                    (const GAME_FLOW_GIVE_ITEM_DATA *)event->data;
                Inv_AddItemNTimes(
                    give_item_data->object_id, give_item_data->quantity);
            }
            break;

        case GFS_REMOVE_GUNS:
            if (level_type != GFL_SAVED
                && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
                g_GameInfo.remove_guns = true;
            }
            break;

        case GFS_REMOVE_SCIONS:
            if (level_type != GFL_SAVED) {
                g_GameInfo.remove_scions = true;
            }
            break;

        case GFS_REMOVE_AMMO:
            if (level_type != GFL_SAVED
                && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
                g_GameInfo.remove_ammo = true;
            }
            break;

        case GFS_REMOVE_MEDIPACKS:
            if (level_type != GFL_SAVED) {
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

        case GFS_LEGACY:
            break;
        }
    }

    return command;
}

GAME_FLOW_COMMAND
GF_StorySoFar(const GAME_FLOW_SEQUENCE *const sequence, int32_t savegame_level)
{
    GAME_FLOW_COMMAND command = { .action = GF_EXIT_TO_TITLE };

    for (int32_t i = 0; i < sequence->length; i++) {
        const GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_INFO("event %d %d", event->type, event->data);

        switch (event->type) {
        case GFS_LOOP_GAME:
        case GFS_STOP_GAME:
        case GFS_LEVEL_STATS:
        case GFS_TOTAL_STATS:
        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE:
        case GFS_GIVE_ITEM:
        case GFS_REMOVE_GUNS:
        case GFS_REMOVE_SCIONS:
        case GFS_REMOVE_AMMO:
        case GFS_REMOVE_MEDIPACKS:
        case GFS_SETUP_BACON_LARA:
        case GFS_LEGACY:
            break;

        case GFS_START_GAME: {
            const int32_t level_num = (int32_t)(intptr_t)event->data;
            if (level_num == savegame_level) {
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            if (g_GameFlow.levels[level_num].level_type == GFL_CUTSCENE) {
                command = GF_LoadLevel(level_num, GFL_CUTSCENE);
                if (command.action != GF_NOOP
                    && command.action != GF_LEVEL_COMPLETE) {
                    return command;
                }
            }
            break;
        }

        case GFS_LOOP_CINE:
            command = GF_PlayCutscene((int32_t)(intptr_t)event->data);
            if (command.action != GF_NOOP
                && command.action != GF_LEVEL_COMPLETE) {
                return command;
            }
            break;

        case GFS_PLAY_FMV:
            FMV_Play((char *)event->data);
            break;

        case GFS_EXIT_TO_TITLE:
            Music_Stop();
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };

        case GFS_EXIT_TO_LEVEL:
            Music_Stop();
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_GAME,
                .param = (int32_t)(intptr_t)event->data & ((1 << 6) - 1),
            };

        case GFS_EXIT_TO_CINE:
            Music_Stop();
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_CINE,
                .param = (int32_t)(intptr_t)event->data & ((1 << 6) - 1),
            };

        case GFS_SET_CAM_ANGLE:
            g_CinePosition.rot = (int32_t)(intptr_t)event->data;
            break;
        case GFS_FLIP_MAP:
            Room_FlipMap();
            break;
        case GFS_PLAY_SYNCED_AUDIO:
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

    return command;
}

GAME_FLOW_COMMAND GF_PlayAvailableStory(int32_t slot_num)
{
    GAME_FLOW_COMMAND command = {
        .action = GF_START_GAME,
        .param = g_GameFlow.first_level_num,
    };

    const int32_t savegame_level = Savegame_GetLevelNumber(slot_num);
    while (1) {
        command = GF_StorySoFar(
            &g_GameFlow.levels[command.param].sequence, savegame_level);
        if (command.action == GF_EXIT_TO_TITLE
            || command.action == GF_EXIT_GAME) {
            break;
        }
    }

    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
}

GAME_FLOW_COMMAND GF_LoadLevel(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    if (!Level_Initialise(level_num)) {
        if (level_num == g_GameFlow.title_level_num) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
        }
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

GAME_FLOW_COMMAND GF_PlayLevel(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    PHASE *const phase = Phase_Game_Create(level_num, level_type);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Game_Destroy(phase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_PlayDemo(const int32_t level_num)
{
    PHASE *const phase = Phase_Demo_Create(level_num);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Demo_Destroy(phase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_PlayCutscene(const int32_t level_num)
{
    PHASE *const phase = Phase_Cutscene_Create(level_num);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Cutscene_Destroy(phase);
    return gf_cmd;
}
