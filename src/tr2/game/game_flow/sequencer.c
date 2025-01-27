#include "game/game_flow/sequencer.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/demo.h"
#include "game/fmv.h"
#include "game/game.h"
#include "game/game_flow.h"
#include "game/music.h"
#include "game/phase.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/enum_map.h>
#include <libtrx/game/game_string_table.h>
#include <libtrx/log.h>

GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    const GAME_FLOW_LEVEL *const level = GF_GetLevel(demo_num, GFL_DEMO);
    if (level == NULL) {
        LOG_ERROR("Missing demo: %d", demo_num);
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL);
}

GAME_FLOW_COMMAND GF_DoCutsceneSequence(const int32_t cutscene_num)
{
    const GAME_FLOW_LEVEL *const level =
        GF_GetLevel(cutscene_num, GFL_CUTSCENE);
    if (level == NULL) {
        LOG_ERROR("Missing cutscene: %d", cutscene_num);
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL);
}

bool GF_DoFrontendSequence(void)
{
    if (g_GameFlow.title_level == NULL) {
        return false;
    }
    const GAME_FLOW_COMMAND gf_cmd =
        GF_InterpretSequence(g_GameFlow.title_level, GFSC_NORMAL);
    return gf_cmd.action == GF_EXIT_GAME;
}

GAME_FLOW_COMMAND GF_DoLevelSequence(
    const GAME_FLOW_LEVEL *const start_level,
    const GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    const GAME_FLOW_LEVEL *current_level = start_level;
    const int32_t level_count = GF_GetLevelCount(current_level->type);
    while (true) {
        LOG_DEBUG(
            "running sequence for level=%d type=%d seq_ctx=%d",
            current_level->num, current_level->type, seq_ctx);
        const GAME_FLOW_COMMAND gf_cmd =
            GF_InterpretSequence(current_level, seq_ctx);
        LOG_DEBUG("sequence finished: %d", gf_cmd.action);

        if (g_GameFlow.single_level >= 0) {
            return gf_cmd;
        }
        if (gf_cmd.action != GF_NOOP && gf_cmd.action != GF_LEVEL_COMPLETE) {
            return gf_cmd;
        }
        if (Game_IsInGym()) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        if (current_level->num >= level_count - 1) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        current_level++;
    }
}

GAME_FLOW_COMMAND GF_RunLevel(
    const GAME_FLOW_LEVEL *const level,
    const GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    if (level->type == GFL_DEMO) {
        return GF_RunDemo(level->num);
    } else if (level->type == GFL_CUTSCENE) {
        return GF_RunCutscene(level->num);
    } else {
        return GF_RunGame(level, seq_ctx);
    }
}

GAME_FLOW_COMMAND GF_InterpretSequence(
    const GAME_FLOW_LEVEL *const level, GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    g_GF_NoFloor = 0;
    g_GF_SunsetEnabled = false;
    g_GF_LaraStartAnim = 0;
    g_GF_RemoveAmmo = false;
    g_GF_RemoveWeapons = false;

    GF_InventoryModifier_Reset();

    g_CineTargetAngle = DEG_90;
    g_GF_NumSecrets = 3;

    int32_t ntracks = 0;
    GAME_FLOW_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };

    const GAME_FLOW_SEQUENCE *const sequence = &level->sequence;
    for (int32_t i = 0; i < sequence->length; i++) {
        const GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_DEBUG(
            "event type=%s(%d) data=0x%x",
            ENUM_MAP_TO_STRING(GAME_FLOW_SEQUENCE_EVENT_TYPE, event->type),
            event->type, event->data);

        switch (event->type) {
        case GFS_EXIT_TO_TITLE:
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };

        case GFS_DISPLAY_PICTURE: {
            const GAME_FLOW_DISPLAY_PICTURE_DATA *const data =
                (GAME_FLOW_DISPLAY_PICTURE_DATA *)event->data;
            PHASE *const phase = Phase_Picture_Create((PHASE_PICTURE_ARGS) {
                .file_name = data->path,
                .display_time = data->display_time,
                .fade_in_time = data->fade_in_time,
                .fade_out_time = data->fade_out_time,
                .display_time_includes_fades = true,
            });
            PhaseExecutor_Run(phase);
            Phase_Picture_Destroy(phase);
            break;
        }

        case GFS_PLAY_LEVEL: {
            if (seq_ctx != GFSC_STORY) {
                gf_cmd = GF_RunLevel(level, seq_ctx);
                if (seq_ctx == GFSC_SAVED) {
                    seq_ctx = GFSC_NORMAL;
                }
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_PLAY_CUTSCENE: {
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

        case GFS_PLAY_MUSIC:
            Music_Play((int32_t)(intptr_t)event->data, MPM_ALWAYS);
            break;

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

        case GFS_LEVEL_STATS: {
            if (seq_ctx != GFSC_NORMAL) {
                break;
            }
            const GAME_FLOW_LEVEL *const current_level = Game_GetCurrentLevel();
            PHASE *const stats_phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
                .background_type = Game_IsInGym() ? BK_TRANSPARENT : BK_OBJECT,
                .level_num = current_level->num,
                .show_final_stats = false,
                .use_bare_style = false,
            });
            gf_cmd = PhaseExecutor_Run(stats_phase);
            Phase_Stats_Destroy(stats_phase);
            break;
        }

        case GFS_LEVEL_COMPLETE: {
            if (seq_ctx != GFSC_NORMAL) {
                break;
            }
            const GAME_FLOW_LEVEL *const current_level = Game_GetCurrentLevel();
            START_INFO *const start = GF_GetResumeInfo(current_level);
            start->stats = g_SaveGame.current_stats;
            start->available = 0;
            const GAME_FLOW_LEVEL *const next_level =
                GF_GetLevel(current_level->num + 1, current_level->type);
            if (next_level != NULL) {
                CreateStartInfo(next_level);
                g_SaveGame.current_level = next_level->num;
            }
            if (next_level == NULL || gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
            gf_cmd = (GAME_FLOW_COMMAND) {
                .action = GF_START_GAME,
                .param = next_level->num,
            };
            break;
        }

        case GFS_ENABLE_SUNSET:
            if (seq_ctx != GFSC_STORY) {
                g_GF_SunsetEnabled = true;
            }
            break;

        case GFS_TOTAL_STATS:
            if (seq_ctx == GFSC_NORMAL) {
                const GAME_FLOW_LEVEL *const current_level =
                    Game_GetCurrentLevel();
                START_INFO *const start = GF_GetResumeInfo(current_level);
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
            } else {
                gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            }
            break;

        case GFS_SET_CAMERA_ANGLE:
            if (seq_ctx != GFSC_SAVED) {
                g_CineTargetAngle = (int16_t)(intptr_t)event->data;
            }
            break;

        case GFS_DISABLE_FLOOR:
            if (seq_ctx != GFSC_STORY) {
                g_GF_NoFloor = (int16_t)(intptr_t)event->data;
            }
            break;

        case GFS_ADD_ITEM:
        case GFS_ADD_SECRET_REWARD:
            const GAME_FLOW_ADD_ITEM_DATA *const data =
                (GAME_FLOW_ADD_ITEM_DATA *)event->data;
            if (seq_ctx != GFSC_STORY) {
                GF_InventoryModifier_Add(
                    data->object_id, data->inv_type, data->qty);
            }
            break;

        case GFS_REMOVE_WEAPONS:
            if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
                g_GF_RemoveWeapons = true;
            }
            break;

        case GFS_REMOVE_AMMO:
            if (seq_ctx != GFSC_STORY && seq_ctx != GFSC_SAVED) {
                g_GF_RemoveAmmo = true;
            }
            break;

        case GFS_SET_START_ANIM:
            if (seq_ctx != GFSC_STORY) {
                g_GF_LaraStartAnim = (int16_t)(intptr_t)event->data;
            }
            break;

        case GFS_SET_NUM_SECRETS:
            if (seq_ctx != GFSC_STORY) {
                g_GF_NumSecrets = (int16_t)(intptr_t)event->data;
            }
            break;
        }
    }

    if (seq_ctx == GFSC_STORY) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return gf_cmd;
}
