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

GAME_FLOW_COMMAND
GF_InterpretSequence(
    const GAME_FLOW_LEVEL *const level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx,
    void *const seq_ctx_arg)
{
    LOG_DEBUG(
        "running sequence for level=%d type=%d seq_ctx=%d", level->num,
        level->type, seq_ctx);

    g_GameInfo.remove_guns = false;
    g_GameInfo.remove_scions = false;
    g_GameInfo.remove_ammo = false;
    g_GameInfo.remove_medipacks = false;

    GAME_FLOW_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };

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
            case GFS_LEVEL_COMPLETE:
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
        case GFS_EXIT_TO_TITLE:
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };

        case GFS_LOAD_LEVEL: {
            if (seq_ctx == GFSC_STORY) {
                const int32_t savegame_level_num =
                    (int32_t)(intptr_t)seq_ctx_arg;
                if (savegame_level_num == level->num) {
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
            } else if (level->type == GFL_DEMO) {
                break;
            } else if (level->type == GFL_CUTSCENE) {
                gf_cmd = GF_LoadLevel(level->num, GFL_CUTSCENE);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            } else {
                const bool result = Game_Start_Legacy(level, seq_ctx);
                if (!result) {
                    g_CurrentLevel = -1;
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
            }
            break;
        }

        case GFS_PLAY_LEVEL:
            if (level->type == GFL_CUTSCENE) {
                if (seq_ctx != GFSC_SAVED) {
                    gf_cmd = GF_RunCutscene((int32_t)(intptr_t)event->data);
                    if (gf_cmd.action != GF_NOOP
                        && gf_cmd.action != GF_LEVEL_COMPLETE) {
                        return gf_cmd;
                    }
                }
            } else if (seq_ctx == GFSC_STORY) {
                const int32_t savegame_level_num =
                    (int32_t)(intptr_t)seq_ctx_arg;
                if (savegame_level_num == level->num) {
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
            } else if (level->type == GFL_DEMO) {
                return GF_RunDemo(level->num);
            } else {
                if (seq_ctx != GFSC_SAVED
                    && level->num != g_GameFlow.first_level_num) {
                    Lara_RevertToPistolsIfNeeded();
                }
                gf_cmd = GF_RunGame(level, seq_ctx);
                if (seq_ctx == GFSC_SAVED || seq_ctx == GFSC_RESTART
                    || seq_ctx == GFSC_SELECT) {
                    seq_ctx = GFSC_NORMAL;
                }
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;

        case GFS_LEVEL_STATS: {
            if (seq_ctx != GFSC_NORMAL) {
                break;
            }
            const GAME_FLOW_LEVEL *const current_level = Game_GetCurrentLevel();
            PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                .background_type = BK_TRANSPARENT,
                .level_num = current_level->num,
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
            if (seq_ctx != GFSC_NORMAL
                || !g_Config.gameplay.enable_total_stats) {
                break;
            }
            PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                .background_type = BK_IMAGE,
                .background_path = event->data,
                .level_num = level->num,
                .show_final_stats = true,
                .use_bare_style = false,
            });
            gf_cmd = PhaseExecutor_Run(phase);
            Phase_Stats_Destroy(phase);
            if (gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
            break;

        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE: {
            if (event->type == GFS_LOADING_SCREEN
                && !g_Config.gameplay.enable_loading_screens) {
                break;
            }
            if (seq_ctx == GFSC_SAVED) {
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
        }

        case GFS_LEVEL_COMPLETE: {
            if (seq_ctx != GFSC_NORMAL) {
                break;
            }
            const GAME_FLOW_LEVEL *const current_level = Game_GetCurrentLevel();
            const GAME_FLOW_LEVEL *const next_level =
                GF_GetLevel(current_level->num + 1, current_level->type);
            if (next_level == NULL) {
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            if (next_level->type == GFL_BONUS
                && !g_GameInfo.bonus_level_unlock) {
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            return (GAME_FLOW_COMMAND) {
                .action = GF_START_GAME,
                .param = next_level->num,
            };
        }

        case GFS_PLAY_CUTSCENE: {
            Music_Stop();
            const int16_t cutscene_num = (int16_t)(intptr_t)event->data;
            if (seq_ctx != GFSC_SAVED) {
                gf_cmd = GF_DoCutsceneSequence(cutscene_num);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_PLAY_FMV: {
            const int16_t fmv_id = (int16_t)(intptr_t)event->data;
            if (seq_ctx != GFSC_SAVED) {
                if (fmv_id < 0 || fmv_id >= g_GameFlow.fmv_count) {
                    LOG_ERROR("Invalid FMV number: %d", fmv_id);
                } else {
                    FMV_Play(g_GameFlow.fmvs[fmv_id].path);
                }
            }
            break;
        }

        case GFS_PLAY_MUSIC:
            Music_Play((int32_t)(intptr_t)event->data);
            break;

        case GFS_SET_CAMERA_ANGLE:
            if (seq_ctx == GFSC_STORY) {
                break;
            }
            g_CinePosition.rot = (int32_t)(intptr_t)event->data;
            break;

        case GFS_FLIP_MAP:
            if (seq_ctx == GFSC_STORY) {
                break;
            }
            Room_FlipMap();
            break;

        case GFS_ADD_ITEM:
            if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
                const GAME_FLOW_ADD_ITEM_DATA *add_item_data =
                    (const GAME_FLOW_ADD_ITEM_DATA *)event->data;
                Inv_AddItemNTimes(
                    add_item_data->object_id, add_item_data->quantity);
            }
            break;

        case GFS_REMOVE_WEAPONS:
            if (seq_ctx == GFSC_STORY || seq_ctx == GFSC_SAVED
                || (g_GameInfo.bonus_flag & GBF_NGPLUS)) {
                break;
            }
            g_GameInfo.remove_guns = true;
            break;

        case GFS_REMOVE_SCIONS:
            if (seq_ctx == GFSC_STORY || seq_ctx == GFSC_SAVED) {
                break;
            }
            g_GameInfo.remove_scions = true;
            break;

        case GFS_REMOVE_AMMO:
            if (seq_ctx == GFSC_STORY || seq_ctx == GFSC_SAVED
                || (g_GameInfo.bonus_flag & GBF_NGPLUS)) {
                break;
            }
            g_GameInfo.remove_ammo = true;
            break;

        case GFS_REMOVE_MEDIPACKS:
            if (seq_ctx == GFSC_STORY || seq_ctx == GFSC_SAVED) {
                break;
            }
            g_GameInfo.remove_medipacks = true;
            break;

        case GFS_MESH_SWAP: {
            if (seq_ctx == GFSC_STORY) {
                break;
            }
            const GAME_FLOW_MESH_SWAP_DATA *const swap_data = event->data;
            Object_SwapMesh(
                swap_data->object1_id, swap_data->object2_id,
                swap_data->mesh_num);
            break;
        }

        case GFS_SETUP_BACON_LARA: {
            if (seq_ctx == GFSC_STORY) {
                break;
            }
            const int32_t anchor_room = (int32_t)(intptr_t)event->data;
            if (!BaconLara_InitialiseAnchor(anchor_room)) {
                LOG_ERROR(
                    "Could not anchor Bacon Lara to room %d", anchor_room);
                return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            break;
        }
        }
    }

    if (seq_ctx == GFSC_STORY) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_PlayAvailableStory(int32_t slot_num)
{
    const int32_t savegame_level = Savegame_GetLevelNumber(slot_num);
    for (int32_t i = g_GameFlow.first_level_num; i <= savegame_level; i++) {
        const GAME_FLOW_COMMAND gf_cmd = GF_InterpretSequence(
            GF_GetLevel(i, GFL_NORMAL), GFSC_STORY,
            (void *)(intptr_t)savegame_level);
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
    return GF_InterpretSequence(
        GF_GetLevel(cutscene_num, GFL_CUTSCENE), GFSC_NORMAL, NULL);
}

GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(
        GF_GetLevel(demo_num, GFL_DEMO), GFSC_NORMAL, NULL);
}
