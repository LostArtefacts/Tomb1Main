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
#define GF_DESCRIPTION_SIZE 256
#define GF_EXPECTED_SIZE 128

static int16_t m_LevelOffsets[200] = {};
static int16_t *m_SequenceBuf = NULL;
static int16_t *m_ScriptTable[MAX_LEVELS] = {};
static int16_t *m_FrontendSequence = NULL;

static void M_ReadStringTable(
    VFILE *file, int32_t count, char ***table, char **buffer,
    uint8_t cypher_code);

static void M_ReadStringTable(
    VFILE *const file, const int32_t count, char ***const table,
    char **const buffer, const uint8_t cypher_code)
{
    VFile_Read(file, m_LevelOffsets, sizeof(int16_t) * count);

    const int16_t buf_size = VFile_ReadS16(file);
    if (buffer != NULL) {
        *buffer = Memory_Alloc(buf_size);
        VFile_Read(file, *buffer, buf_size);

        for (int32_t i = 0; i < buf_size; i++) {
            (*buffer)[i] ^= cypher_code;
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

    char description[GF_DESCRIPTION_SIZE];
    VFile_Read(file, description, GF_DESCRIPTION_SIZE);

    if (VFile_ReadS16(file) != GF_EXPECTED_SIZE) {
        return false;
    }
    VFile_Skip(file, 64);
    const uint16_t num_levels = VFile_ReadU16(file);
    const uint16_t num_pictures = VFile_ReadU16(file);
    const uint16_t num_titles = VFile_ReadU16(file);
    const uint16_t num_fmvs = VFile_ReadU16(file);
    const uint16_t num_cutscenes = VFile_ReadU16(file);
    VFile_Skip(file, 46);

    const uint8_t cypher_code = VFile_ReadU8(file);
    VFile_Skip(file, 7);

    g_GF_LevelNames = Memory_Alloc(sizeof(char *) * num_levels);
    M_ReadStringTable(file, num_levels, NULL, NULL, 0);
    // picture filename strings
    M_ReadStringTable(file, num_pictures, NULL, NULL, 0);
    M_ReadStringTable(
        file, num_titles, &g_GF_TitleFileNames, &g_GF_TitleFileNamesBuf,
        cypher_code);
    M_ReadStringTable(file, num_fmvs, NULL, NULL, 0);
    M_ReadStringTable(
        file, num_levels, &g_GF_LevelFileNames, &g_GF_LevelFileNamesBuf,
        cypher_code);
    M_ReadStringTable(
        file, num_cutscenes, &g_GF_CutsceneFileNames,
        &g_GF_CutsceneFileNamesBuf, cypher_code);

    VFile_Read(file, &m_LevelOffsets, sizeof(int16_t) * (num_levels + 1));
    {
        const int16_t size = VFile_ReadS16(file);
        m_SequenceBuf = Memory_Alloc(size);
        VFile_Read(file, m_SequenceBuf, size);
    }

    m_FrontendSequence = m_SequenceBuf;
    for (int32_t i = 0; i < num_levels; i++) {
        m_ScriptTable[i] = m_SequenceBuf + (m_LevelOffsets[i + 1] / 2);
    }

    g_LegacyLevelCount = num_levels;

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
        if (current_level > GF_GetLevelCount() - 1) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }

        const int16_t *const ptr = m_ScriptTable[current_level];
        const GAME_FLOW_COMMAND gf_cmd = GF_InterpretSequence(ptr, type);
        current_level++;

        if (g_GameFlow.single_level >= 0) {
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
                if (ptr[1] >= g_GameFlow.fmv_count) {
                    LOG_ERROR("Invalid FMV number: %d", ptr[1]);
                } else {
                    FMV_Play(g_GameFlow.fmvs[ptr[1]].path);
                    if (g_IsGameToExit) {
                        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
                    }
                }
            }
            ptr += 2;
            break;

        case GFE_START_LEVEL:
            if (ptr[1] > GF_GetLevelCount()) {
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
