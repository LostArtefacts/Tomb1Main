#include "game/game_flow/sequencer.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/demo.h"
#include "game/fmv.h"
#include "game/game_flow.h"
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
    return GF_InterpretSequence(&g_GameFlow.demos[demo_num].sequence, GFL_DEMO);
}

GAME_FLOW_COMMAND GF_StartGame(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    if (level_type == GFL_DEMO) {
        PHASE *const demo_phase = Phase_Demo_Create(level_num);
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(demo_phase);
        Phase_Demo_Destroy(demo_phase);
        return gf_cmd;
    } else {
        PHASE *const phase = Phase_Game_Create(level_num, level_type);
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
        Phase_Game_Destroy(phase);
        return gf_cmd;
    }
}

bool GF_DoFrontendSequence(void)
{
    GameStringTable_Apply(-1);
    if (g_GameFlow.title_level == NULL) {
        return false;
    }
    const GAME_FLOW_COMMAND gf_cmd =
        GF_InterpretSequence(&g_GameFlow.title_level->sequence, GFL_NORMAL);
    return gf_cmd.action == GF_EXIT_GAME;
}

GAME_FLOW_COMMAND GF_InterpretSequence(
    const GAME_FLOW_SEQUENCE *sequence, GAME_FLOW_LEVEL_TYPE type)
{
    g_GF_NoFloor = 0;
    g_GF_SunsetEnabled = false;
    g_GF_LaraStartAnim = 0;
    g_GF_RemoveAmmo = false;
    g_GF_RemoveWeapons = false;

    GF_InventoryModifier_Reset();

    g_GF_MusicTracks[0] = 2;
    g_CineTargetAngle = DEG_90;
    g_GF_NumSecrets = 3;

    int32_t ntracks = 0;
    GAME_FLOW_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };

    for (int32_t i = 0; i < sequence->length; i++) {
        const GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_DEBUG(
            "event type=%s(%d) data=0x%x",
            ENUM_MAP_TO_STRING(GAME_FLOW_SEQUENCE_EVENT_TYPE, event->type),
            event->type, event->data);

        switch (event->type) {
        case GFS_DISPLAY_PICTURE: {
            const GFS_DISPLAY_PICTURE_DATA *const data =
                (GFS_DISPLAY_PICTURE_DATA *)event->data;
            PHASE *const phase = Phase_Picture_Create((PHASE_PICTURE_ARGS) {
                .file_name = data->path,
                .display_time = data->duration,
                .fade_in_time = 1.0,
                .fade_out_time = 1.0 / 3.0,
                .display_time_includes_fades = true,
            });
            PhaseExecutor_Run(phase);
            Phase_Picture_Destroy(phase);
            break;
        }

        case GFS_PLAY_LEVEL: {
            const int16_t level_num = (int16_t)(intptr_t)event->data;
            if (level_num < 0 || level_num > GF_GetLevelCount()) {
                LOG_ERROR("Invalid level number: %d", level_num);
                gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            } else if (type != GFL_STORY) {
                if (type == GFL_MID_STORY) {
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
                gf_cmd = GF_StartGame(level_num, type);
                if (type == GFL_SAVED) {
                    type = GFL_NORMAL;
                }
                if (gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_PLAY_CUTSCENE: {
            const int16_t cutscene_num = (int16_t)(intptr_t)event->data;
            if (type != GFL_SAVED) {
                if (cutscene_num < 0 || cutscene_num >= GF_GetCutsceneCount()) {
                    LOG_ERROR("Invalid cutscene number: %d", cutscene_num);
                }
                gf_cmd = GF_PlayCutscene(cutscene_num);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            break;
        }

        case GFS_PLAY_FMV: {
            const int16_t fmv_num = (int16_t)(intptr_t)event->data;
            if (type != GFL_SAVED) {
                if (fmv_num >= g_GameFlow.fmv_count) {
                    LOG_ERROR("Invalid FMV number: %d", fmv_num);
                } else {
                    FMV_Play(g_GameFlow.fmvs[fmv_num].path);
                    if (Shell_IsExiting()) {
                        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
                    }
                }
            }
            break;
        }

        case GFS_LEVEL_COMPLETE:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                START_INFO *const start = &g_SaveGame.start[g_CurrentLevel];
                start->stats = g_SaveGame.current_stats;

                if (g_GameFlow.level_complete_track != MX_INACTIVE) {
                    Music_Play(g_GameFlow.level_complete_track, MPM_ALWAYS);
                }
                PHASE *const stats_phase =
                    Phase_Stats_Create((PHASE_STATS_ARGS) {
                        .background_type = BK_OBJECT,
                        .show_final_stats = false,
                        .level_num = g_CurrentLevel,
                        .use_bare_style = false,
                    });
                gf_cmd = PhaseExecutor_Run(stats_phase);
                Phase_Stats_Destroy(stats_phase);

                CreateStartInfo(g_CurrentLevel + 1);
                g_SaveGame.current_level = g_CurrentLevel + 1;
                start->available = 0;

                if (gf_cmd.action != GF_NOOP) {
                    return gf_cmd;
                }
                gf_cmd = (GAME_FLOW_COMMAND) {
                    .action = GF_START_GAME,
                    .param = g_CurrentLevel + 1,
                };
            }
            break;

        case GFS_SET_MUSIC_TRACK: {
            const int16_t music_track_id = (int16_t)(intptr_t)event->data;
            g_GF_MusicTracks[ntracks] = music_track_id;
            Game_SetCutsceneTrack(music_track_id);
            ntracks++;
            break;
        }

        case GFS_ENABLE_SUNSET:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_SunsetEnabled = true;
            }
            break;

        case GFS_GAME_COMPLETE:
            START_INFO *const start = &g_SaveGame.start[g_CurrentLevel];
            start->stats = g_SaveGame.current_stats;
            g_SaveGame.bonus_flag = true;
            gf_cmd = DisplayCredits();
            break;

        case GFS_SET_CAMERA_ANGLE:
            if (type != GFL_SAVED) {
                g_CineTargetAngle = (int16_t)(intptr_t)event->data;
            }
            break;

        case GFS_DISABLE_FLOOR:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_NoFloor = (int16_t)(intptr_t)event->data;
            }
            break;

        case GFS_ADD_ITEM:
        case GFS_ADD_SECRET_REWARD:
            const GFS_ADD_ITEM_DATA *const data =
                (GFS_ADD_ITEM_DATA *)event->data;
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                GF_InventoryModifier_Add(
                    data->object_id, data->inv_type, data->qty);
            }
            break;

        case GFS_REMOVE_WEAPONS:
            if (type != GFL_STORY && type != GFL_MID_STORY
                && type != GFL_SAVED) {
                g_GF_RemoveWeapons = true;
            }
            break;

        case GFS_REMOVE_AMMO:
            if (type != GFL_STORY && type != GFL_MID_STORY
                && type != GFL_SAVED) {
                g_GF_RemoveAmmo = true;
            }
            break;

        case GFS_SET_START_ANIM:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_LaraStartAnim = (int16_t)(intptr_t)event->data;
            }
            break;

        case GFS_SET_NUM_SECRETS:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_NumSecrets = (int16_t)(intptr_t)event->data;
            }
            break;
        }
    }

    if (type == GFL_STORY || type == GFL_MID_STORY) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_DoLevelSequence(
    const int32_t start_level, const GAME_FLOW_LEVEL_TYPE type)
{
    GameStringTable_Apply(start_level);
    int32_t current_level = start_level;
    while (true) {
        if (current_level > GF_GetLevelCount() - 1) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }

        LOG_DEBUG("running sequence for level=%d type=%d", current_level, type);
        const GAME_FLOW_COMMAND gf_cmd = GF_InterpretSequence(
            &g_GameFlow.levels[current_level].sequence, type);
        LOG_DEBUG("sequence finished");
        current_level++;

        if (g_GameFlow.single_level >= 0) {
            return gf_cmd;
        }
        if (gf_cmd.action != GF_LEVEL_COMPLETE) {
            return gf_cmd;
        }
    }
}
