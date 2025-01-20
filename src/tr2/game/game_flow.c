#include "game/game_flow.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/fmv.h"
#include "game/music.h"
#include "game/phase.h"
#include "global/vars.h"

#include <libtrx/benchmark.h>
#include <libtrx/filesystem.h>
#include <libtrx/game/game_string_table.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/virtual_file.h>

#define GF_CURRENT_VERSION 3

static int16_t m_LevelOffsets[200] = {};
static int16_t *m_SequenceBuf = NULL;
static int16_t *m_ScriptTable[MAX_LEVELS] = {};
static int16_t *m_FrontendSequence = NULL;

static void M_ReadStringTable(
    VFILE *file, int32_t count, char ***table, char **buffer);

static void M_ReadStringTable(
    VFILE *const file, const int32_t count, char ***const table,
    char **const buffer)
{
    VFile_Read(file, m_LevelOffsets, sizeof(int16_t) * count);

    const int16_t buf_size = VFile_ReadS16(file);
    if (buffer != NULL) {
        *buffer = Memory_Alloc(buf_size);
        VFile_Read(file, *buffer, buf_size);

        if (g_GameFlowLegacy.cyphered_strings) {
            for (int32_t i = 0; i < buf_size; i++) {
                (*buffer)[i] ^= g_GameFlowLegacy.cypher_code;
            }
        }

        if (table != NULL) {
            *table = Memory_Alloc(sizeof(char *) * count);
            for (int32_t i = 0; i < count; i++) {
                const int32_t offset = m_LevelOffsets[i];
                (*table)[i] = &(*buffer)[offset];
            }
        }
    } else {
        VFile_Skip(file, buf_size);
    }
}

// TODO: inline me into GF_LoadScriptFile
bool GF_LoadFromFile(const char *const file_name)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    const char *full_path = File_GetFullPath(file_name);
    VFILE *const file = VFile_CreateFromPath(full_path);
    Memory_FreePointer(&full_path);
    if (file == NULL) {
        return false;
    }

    const int32_t script_version = VFile_ReadS32(file);
    if (script_version != GF_CURRENT_VERSION) {
        return false;
    }

    char description[256];
    VFile_Read(file, description, 256);

    if (VFile_ReadS16(file) != sizeof(GAME_FLOW_LEGACY)) {
        return false;
    }
    g_GameFlowLegacy.first_option = VFile_ReadS32(file);
    g_GameFlowLegacy.title_replace = VFile_ReadS32(file);
    g_GameFlowLegacy.on_death_demo_mode = VFile_ReadS32(file);
    g_GameFlowLegacy.on_death_in_game = VFile_ReadS32(file);
    g_GameFlowLegacy.no_input_time = VFile_ReadS32(file);
    g_GameFlowLegacy.on_demo_interrupt = VFile_ReadS32(file);
    g_GameFlowLegacy.on_demo_end = VFile_ReadS32(file);
    VFile_Skip(file, 36);
    g_GameFlowLegacy.num_levels = VFile_ReadU16(file);
    g_GameFlowLegacy.num_pictures = VFile_ReadU16(file);
    g_GameFlowLegacy.num_titles = VFile_ReadU16(file);
    g_GameFlowLegacy.num_fmvs = VFile_ReadU16(file);
    g_GameFlowLegacy.num_cutscenes = VFile_ReadU16(file);
    g_GameFlowLegacy.num_demos = VFile_ReadU16(file);
    g_GameFlowLegacy.title_track = VFile_ReadU16(file);
    g_GameFlowLegacy.single_level = VFile_ReadS16(file);
    VFile_Skip(file, 32);

    const uint16_t flags = VFile_ReadU16(file);
    // clang-format off
    g_GameFlowLegacy.demo_version              = flags & 0x0001 ? 1 : 0;
    g_GameFlowLegacy.title_disabled            = flags & 0x0002 ? 1 : 0;
    g_GameFlowLegacy.cheat_mode_check_disabled = flags & 0x0004 ? 1 : 0;
    g_GameFlowLegacy.load_save_disabled        = flags & 0x0010 ? 1 : 0;
    g_GameFlowLegacy.screen_sizing_disabled    = flags & 0x0020 ? 1 : 0;
    g_GameFlowLegacy.lockout_option_ring       = flags & 0x0040 ? 1 : 0;
    g_GameFlowLegacy.dozy_cheat_enabled        = flags & 0x0080 ? 1 : 0;
    g_GameFlowLegacy.cyphered_strings          = flags & 0x0100 ? 1 : 0;
    g_GameFlowLegacy.gym_enabled               = flags & 0x0200 ? 1 : 0;
    g_GameFlowLegacy.play_any_level            = flags & 0x0400 ? 1 : 0;
    g_GameFlowLegacy.cheat_enable              = flags & 0x0800 ? 1 : 0;
    // clang-format on
    VFile_Skip(file, 6);

    g_GameFlowLegacy.cypher_code = VFile_ReadU8(file);
    g_GameFlowLegacy.language = VFile_ReadU8(file);
    g_GameFlowLegacy.secret_track = VFile_ReadU8(file);
    g_GameFlowLegacy.level_complete_track = VFile_ReadU8(file);
    VFile_Skip(file, 4);

    g_GF_LevelNames =
        Memory_Alloc(sizeof(char *) * g_GameFlowLegacy.num_levels);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    // picture filename strings
    M_ReadStringTable(file, g_GameFlowLegacy.num_pictures, NULL, NULL);
    M_ReadStringTable(
        file, g_GameFlowLegacy.num_titles, &g_GF_TitleFileNames,
        &g_GF_TitleFileNamesBuf);
    M_ReadStringTable(
        file, g_GameFlowLegacy.num_fmvs, &g_GF_FMVFilenames,
        &g_GF_FMVFilenamesBuf);
    M_ReadStringTable(
        file, g_GameFlowLegacy.num_levels, &g_GF_LevelFileNames,
        &g_GF_LevelFileNamesBuf);
    M_ReadStringTable(
        file, g_GameFlowLegacy.num_cutscenes, &g_GF_CutsceneFileNames,
        &g_GF_CutsceneFileNamesBuf);

    VFile_Read(
        file, &m_LevelOffsets,
        sizeof(int16_t) * (g_GameFlowLegacy.num_levels + 1));
    {
        const int16_t size = VFile_ReadS16(file);
        m_SequenceBuf = Memory_Alloc(size);
        VFile_Read(file, m_SequenceBuf, size);
    }

    m_FrontendSequence = m_SequenceBuf;
    for (int32_t i = 0; i < g_GameFlowLegacy.num_levels; i++) {
        m_ScriptTable[i] = m_SequenceBuf + (m_LevelOffsets[i + 1] / 2);
    }

    VFile_Read(
        file, g_GF_ValidDemos, sizeof(int16_t) * g_GameFlowLegacy.num_demos);

    // game strings
    if (VFile_ReadS16(file) != 89) {
        return false;
    }
    M_ReadStringTable(file, 89, NULL, NULL);

    // pc strings
    M_ReadStringTable(file, 41, NULL, NULL);

    // puzzle 1-4 strings
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    // pickup 1-2 strings
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    // key 1-4 strings
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);
    M_ReadStringTable(file, g_GameFlowLegacy.num_levels, NULL, NULL);

    VFile_Close(file);
    Benchmark_End(benchmark, NULL);
    return true;
}

bool GF_LoadScriptFile(const char *const fname)
{
    g_GF_SunsetEnabled = false;

    if (!GF_LoadFromFile(fname)) {
        return false;
    }

    g_GameFlowLegacy.level_complete_track = MX_END_OF_LEVEL;
    return true;
}

bool GF_DoFrontendSequence(void)
{
    GameStringTable_Apply(-1);
    const GAME_FLOW_COMMAND gf_cmd =
        GF_InterpretSequence(m_FrontendSequence, GFL_NORMAL);
    return gf_cmd.action == GF_EXIT_GAME;
}

GAME_FLOW_COMMAND GF_DoLevelSequence(
    const int32_t start_level, const GAME_FLOW_LEVEL_TYPE type)
{
    GameStringTable_Apply(start_level);
    int32_t current_level = start_level;
    while (true) {
        if (current_level > g_GameFlowLegacy.num_levels - 1) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }

        const int16_t *const ptr = m_ScriptTable[current_level];
        const GAME_FLOW_COMMAND gf_cmd = GF_InterpretSequence(ptr, type);
        current_level++;

        if (g_GameFlowLegacy.single_level >= 0) {
            return gf_cmd;
        }
        if (gf_cmd.action != GF_LEVEL_COMPLETE) {
            return gf_cmd;
        }
    }
}

GAME_FLOW_COMMAND GF_InterpretSequence(
    const int16_t *ptr, GAME_FLOW_LEVEL_TYPE type)
{
    g_GF_NoFloor = 0;
    g_GF_DeadlyWater = false;
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

    while (*ptr != GFE_END_SEQ) {
        switch (*ptr) {
        case GFE_PICTURE:
            ptr += 2;
            break;

        case GFE_LIST_START:
        case GFE_LIST_END:
            ptr++;
            break;

        case GFE_PLAY_FMV:
            if (type != GFL_SAVED) {
                FMV_Play(g_GF_FMVFilenames[ptr[1]]);
                if (g_IsGameToExit) {
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
                }
            }
            ptr += 2;
            break;

        case GFE_START_LEVEL:
            if (ptr[1] > g_GameFlowLegacy.num_levels) {
                gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            } else if (type != GFL_STORY) {
                if (type == GFL_MID_STORY) {
                    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                }
                gf_cmd = GF_StartGame(ptr[1], type);
                if (type == GFL_SAVED) {
                    type = GFL_NORMAL;
                }
                if (gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            ptr += 2;
            break;

        case GFE_CUTSCENE:
            if (type != GFL_SAVED) {
                PHASE *const cutscene_phase = Phase_Cutscene_Create(ptr[1]);
                gf_cmd = PhaseExecutor_Run(cutscene_phase);
                Phase_Cutscene_Destroy(cutscene_phase);
                if (gf_cmd.action != GF_NOOP
                    && gf_cmd.action != GF_LEVEL_COMPLETE) {
                    return gf_cmd;
                }
            }
            ptr += 2;
            break;

        case GFE_LEVEL_COMPLETE:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                START_INFO *const start = &g_SaveGame.start[g_CurrentLevel];
                start->stats = g_SaveGame.current_stats;

                Music_Play(g_GameFlowLegacy.level_complete_track, MPM_ALWAYS);
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
            ptr++;
            break;

        case GFE_DEMO_PLAY:
            if (type != GFL_SAVED && type != GFL_STORY
                && type != GFL_MID_STORY) {
                return GF_StartGame(ptr[1], GFL_DEMO);
            }
            ptr += 2;
            break;

        case GFE_JUMP_TO_SEQ:
            ptr += 2;
            break;

        case GFE_SET_TRACK:
            g_GF_MusicTracks[ntracks] = ptr[1];
            Game_SetCutsceneTrack(ptr[1]);
            ntracks++;
            ptr += 2;
            break;

        case GFE_SUNSET:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_SunsetEnabled = true;
            }
            ptr++;
            break;

        case GFE_LOADING_PIC:
            ptr += 2;
            break;

        case GFE_DEADLY_WATER:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_DeadlyWater = true;
            }
            ptr++;
            break;

        case GFE_REMOVE_WEAPONS:
            if (type != GFL_STORY && type != GFL_MID_STORY
                && type != GFL_SAVED) {
                g_GF_RemoveWeapons = true;
            }
            ptr++;
            break;

        case GFE_GAME_COMPLETE:
            START_INFO *const start = &g_SaveGame.start[g_CurrentLevel];
            start->stats = g_SaveGame.current_stats;
            g_SaveGame.bonus_flag = true;
            gf_cmd = DisplayCredits();
            ptr++;
            break;

        case GFE_CUT_ANGLE:
            if (type != GFL_SAVED) {
                g_CineTargetAngle = ptr[1];
            }
            ptr += 2;
            break;

        case GFE_NO_FLOOR:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_NoFloor = ptr[1];
            }
            ptr += 2;
            break;

        case GFE_ADD_TO_INV:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                if (ptr[1] < 1000) {
                    GF_InventoryModifier_Add(ptr[1], GF_INV_SECRET);
                } else if (type != GFL_SAVED) {
                    GF_InventoryModifier_Add(ptr[1] - 1000, GF_INV_REGULAR);
                }
            }
            ptr += 2;
            break;

        case GFE_START_ANIM:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_LaraStartAnim = ptr[1];
            }
            ptr += 2;
            break;

        case GFE_NUM_SECRETS:
            if (type != GFL_STORY && type != GFL_MID_STORY) {
                g_GF_NumSecrets = ptr[1];
            }
            ptr += 2;
            break;

        case GFE_KILL_TO_COMPLETE:
            ptr++;
            break;

        case GFE_REMOVE_AMMO:
            if (type != GFL_STORY && type != GFL_MID_STORY
                && type != GFL_SAVED) {
                g_GF_RemoveAmmo = true;
            }
            ptr++;
            break;

        default:
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
        }
    }

    if (type == GFL_STORY || type == GFL_MID_STORY) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_TranslateScriptCommand(const uint16_t dir)
{
    GAME_FLOW_ACTION action = GF_NOOP;

    // clang-format off
    switch (dir & 0xFF00) {
    case 0x0000: action = GF_START_GAME; break;
    case 0x0100: action = GF_START_SAVED_GAME; break;
    case 0x0200: action = GF_START_CINE; break;
    case 0x0300: action = GF_START_FMV; break;
    case 0x0400: action = GF_START_DEMO; break;
    case 0x0500: action = GF_EXIT_TO_TITLE; break;
    case 0x0600: action = GF_LEVEL_COMPLETE; break;
    case 0x0700: action = GF_EXIT_GAME; break;
    case 0x0800: action = GF_EXIT_TO_TITLE; break;
    default:
        LOG_ERROR("Unknown gameflow dir: %x", dir);
        break;
    }
    // clang-format on

    return (GAME_FLOW_COMMAND) {
        .action = action,
        .param = (int8_t)(dir & 0xFF),
    };
}
