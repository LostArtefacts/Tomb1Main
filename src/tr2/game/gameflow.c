#include "game/gameflow.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "decomp/stats.h"
#include "game/demo.h"
#include "game/fmv.h"
#include "game/game.h"
#include "game/gun/gun.h"
#include "game/inventory.h"
#include "game/inventory_ring.h"
#include "game/music.h"
#include "game/objects/vars.h"
#include "game/overlay.h"
#include "game/phase.h"
#include "gameflow/gameflow_new.h"
#include "global/vars.h"

#include <libtrx/benchmark.h>
#include <libtrx/config.h>
#include <libtrx/filesystem.h>
#include <libtrx/game/objects/names.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/virtual_file.h>

#include <stdio.h>

#define GF_CURRENT_VERSION 3

static int16_t m_LevelOffsets[200] = {};
static int16_t *m_SequenceBuf = NULL;
static int16_t *m_ScriptTable[MAX_LEVELS] = {};
static int16_t *m_FrontendSequence = NULL;
static int8_t m_SecretInvItems[GF_ADD_INV_NUMBER_OF] = {};
static int8_t m_Add2InvItems[GF_ADD_INV_NUMBER_OF];

static void M_ReadStringTable(
    VFILE *file, int32_t count, char ***table, char **buffer);

static GF_ADD_INV M_ModifyInventory_GetGunAdder(LARA_GUN_TYPE gun_type);
static GF_ADD_INV M_ModifyInventory_GetAmmoAdder(LARA_GUN_TYPE gun_type);
static GF_ADD_INV M_ModifyInventory_GetItemAdder(GAME_OBJECT_ID object_id);
static void M_ModifyInventory_GunOrAmmo(
    START_INFO *start, int32_t type, LARA_GUN_TYPE gun_type);
static void M_ModifyInventory_Item(int32_t type, GAME_OBJECT_ID object_id);

static void M_ReadStringTable(
    VFILE *const file, const int32_t count, char ***const table,
    char **const buffer)
{
    VFile_Read(file, m_LevelOffsets, sizeof(int16_t) * count);

    const int16_t buf_size = VFile_ReadS16(file);
    if (buffer != NULL) {
        *buffer = Memory_Alloc(buf_size);
        VFile_Read(file, *buffer, buf_size);

        if (g_GameFlow.cyphered_strings) {
            for (int32_t i = 0; i < buf_size; i++) {
                (*buffer)[i] ^= g_GameFlow.cypher_code;
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

static GF_ADD_INV M_ModifyInventory_GetGunAdder(const LARA_GUN_TYPE gun_type)
{
    // clang-format off
    switch (gun_type) {
    case LGT_PISTOLS: return GF_ADD_INV_PISTOLS;
    case LGT_MAGNUMS: return GF_ADD_INV_MAGNUMS;
    case LGT_UZIS:    return GF_ADD_INV_UZIS;
    case LGT_SHOTGUN: return GF_ADD_INV_SHOTGUN;
    case LGT_HARPOON: return GF_ADD_INV_HARPOON;
    case LGT_M16:     return GF_ADD_INV_M16;
    case LGT_GRENADE: return GF_ADD_INV_GRENADE;
    default:          return (GF_ADD_INV)-1;
    }
    // clang-format on
}

static GF_ADD_INV M_ModifyInventory_GetAmmoAdder(const LARA_GUN_TYPE gun_type)
{
    // clang-format off
    switch (gun_type) {
    case LGT_PISTOLS: return GF_ADD_INV_PISTOL_AMMO;
    case LGT_MAGNUMS: return GF_ADD_INV_MAGNUM_AMMO;
    case LGT_UZIS:    return GF_ADD_INV_UZI_AMMO;
    case LGT_SHOTGUN: return GF_ADD_INV_SHOTGUN_AMMO;
    case LGT_HARPOON: return GF_ADD_INV_HARPOON_AMMO;
    case LGT_M16:     return GF_ADD_INV_M16_AMMO;
    case LGT_GRENADE: return GF_ADD_INV_GRENADE_AMMO;
    default:          return (GF_ADD_INV)-1;
    }
    // clang-format on
}

static GF_ADD_INV M_ModifyInventory_GetItemAdder(const GAME_OBJECT_ID object_id)
{
    // clang-format off
    switch (object_id) {
    case O_FLARE_ITEM:          return GF_ADD_INV_FLARES;
    case O_SMALL_MEDIPACK_ITEM: return GF_ADD_INV_SMALL_MEDI;
    case O_LARGE_MEDIPACK_ITEM: return GF_ADD_INV_LARGE_MEDI;
    case O_PICKUP_ITEM_1:       return GF_ADD_INV_PICKUP_1;
    case O_PICKUP_ITEM_2:       return GF_ADD_INV_PICKUP_2;
    case O_PUZZLE_ITEM_1:       return GF_ADD_INV_PUZZLE_1;
    case O_PUZZLE_ITEM_2:       return GF_ADD_INV_PUZZLE_2;
    case O_PUZZLE_ITEM_3:       return GF_ADD_INV_PUZZLE_3;
    case O_PUZZLE_ITEM_4:       return GF_ADD_INV_PUZZLE_4;
    case O_KEY_ITEM_1:          return GF_ADD_INV_KEY_1;
    case O_KEY_ITEM_2:          return GF_ADD_INV_KEY_2;
    case O_KEY_ITEM_3:          return GF_ADD_INV_KEY_3;
    case O_KEY_ITEM_4:          return GF_ADD_INV_KEY_4;
    default:                    return (GF_ADD_INV)-1;
    }
    // clang-format on
}

static void M_ModifyInventory_GunOrAmmo(
    START_INFO *const start, const int32_t type, const LARA_GUN_TYPE gun_type)
{
    const GAME_OBJECT_ID gun_item = Gun_GetGunObject(gun_type);
    const GAME_OBJECT_ID ammo_item = Gun_GetAmmoObject(gun_type);
    const int32_t ammo_qty = Gun_GetAmmoQuantity(gun_type);
    AMMO_INFO *const ammo_info = Gun_GetAmmoInfo(gun_type);

    const GF_ADD_INV gun_adder = M_ModifyInventory_GetGunAdder(gun_type);
    const GF_ADD_INV ammo_adder = M_ModifyInventory_GetAmmoAdder(gun_type);

    if (Inv_RequestItem(gun_item)) {
        if (type == 1) {
            ammo_info->ammo += ammo_qty * m_SecretInvItems[ammo_adder];
            for (int32_t i = 0; i < m_SecretInvItems[ammo_adder]; i++) {
                Overlay_AddDisplayPickup(ammo_item);
            }
        } else if (type == 0) {
            ammo_info->ammo += ammo_qty * m_Add2InvItems[ammo_adder];
        }
    } else if (
        (type == 0 && m_Add2InvItems[gun_adder])
        || (type == 1 && m_SecretInvItems[gun_adder])) {

        // clang-format off
        // TODO: consider moving this to Inv_AddItem
        switch (gun_type) {
        case LGT_PISTOLS: start->has_pistols = 1; break;
        case LGT_MAGNUMS: start->has_magnums = 1; break;
        case LGT_UZIS:    start->has_uzis = 1;    break;
        case LGT_SHOTGUN: start->has_shotgun = 1; break;
        case LGT_HARPOON: start->has_harpoon = 1; break;
        case LGT_M16:     start->has_m16 = 1;     break;
        case LGT_GRENADE: start->has_grenade = 1; break;
        default: break;
        }
        // clang-format on

        Inv_AddItem(gun_item);

        if (type == 1) {
            ammo_info->ammo += ammo_qty * m_SecretInvItems[ammo_adder];
            Overlay_AddDisplayPickup(gun_item);
            for (int32_t i = 0; i < m_SecretInvItems[ammo_adder]; i++) {
                Overlay_AddDisplayPickup(ammo_item);
            }
        } else if (type == 0) {
            ammo_info->ammo += ammo_qty * m_Add2InvItems[ammo_adder];
        }
    } else if (type == 1) {
        for (int32_t i = 0; i < m_SecretInvItems[ammo_adder]; i++) {
            Inv_AddItem(ammo_item);
            Overlay_AddDisplayPickup(ammo_item);
        }
    } else if (type == 0) {
        for (int32_t i = 0; i < m_Add2InvItems[ammo_adder]; i++) {
            Inv_AddItem(ammo_item);
        }
    }
}

static void M_ModifyInventory_Item(
    const int32_t type, const GAME_OBJECT_ID object_id)
{
    const GF_ADD_INV item_adder = M_ModifyInventory_GetItemAdder(object_id);
    int32_t qty = 0;
    if (type == 1) {
        qty = m_SecretInvItems[item_adder];
    } else if (type == 0) {
        qty = m_Add2InvItems[item_adder];
    }

    for (int32_t i = 0; i < qty; i++) {
        Inv_AddItem(object_id);
        if (type == 1) {
            Overlay_AddDisplayPickup(object_id);
        }
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

    if (VFile_ReadS16(file) != sizeof(GAME_FLOW)) {
        return false;
    }
    g_GameFlow.first_option = VFile_ReadS32(file);
    g_GameFlow.title_replace = VFile_ReadS32(file);
    g_GameFlow.on_death_demo_mode = VFile_ReadS32(file);
    g_GameFlow.on_death_in_game = VFile_ReadS32(file);
    g_GameFlow.no_input_time = VFile_ReadS32(file);
    g_GameFlow.on_demo_interrupt = VFile_ReadS32(file);
    g_GameFlow.on_demo_end = VFile_ReadS32(file);
    VFile_Skip(file, 36);
    g_GameFlow.num_levels = VFile_ReadU16(file);
    g_GameFlow.num_pictures = VFile_ReadU16(file);
    g_GameFlow.num_titles = VFile_ReadU16(file);
    g_GameFlow.num_fmvs = VFile_ReadU16(file);
    g_GameFlow.num_cutscenes = VFile_ReadU16(file);
    g_GameFlow.num_demos = VFile_ReadU16(file);
    g_GameFlow.title_track = VFile_ReadU16(file);
    g_GameFlow.single_level = VFile_ReadS16(file);
    VFile_Skip(file, 32);

    const uint16_t flags = VFile_ReadU16(file);
    // clang-format off
    g_GameFlow.demo_version              = flags & 0x0001 ? 1 : 0;
    g_GameFlow.title_disabled            = flags & 0x0002 ? 1 : 0;
    g_GameFlow.cheat_mode_check_disabled = flags & 0x0004 ? 1 : 0;
    g_GameFlow.load_save_disabled        = flags & 0x0010 ? 1 : 0;
    g_GameFlow.screen_sizing_disabled    = flags & 0x0020 ? 1 : 0;
    g_GameFlow.lockout_option_ring       = flags & 0x0040 ? 1 : 0;
    g_GameFlow.dozy_cheat_enabled        = flags & 0x0080 ? 1 : 0;
    g_GameFlow.cyphered_strings          = flags & 0x0100 ? 1 : 0;
    g_GameFlow.gym_enabled               = flags & 0x0200 ? 1 : 0;
    g_GameFlow.play_any_level            = flags & 0x0400 ? 1 : 0;
    g_GameFlow.cheat_enable              = flags & 0x0800 ? 1 : 0;
    // clang-format on
    VFile_Skip(file, 6);

    g_GameFlow.cypher_code = VFile_ReadU8(file);
    g_GameFlow.language = VFile_ReadU8(file);
    g_GameFlow.secret_track = VFile_ReadU8(file);
    g_GameFlow.level_complete_track = VFile_ReadU8(file);
    VFile_Skip(file, 4);

    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_LevelNames, &g_GF_LevelNamesBuf);
    // picture filename strings
    M_ReadStringTable(file, g_GameFlow.num_pictures, NULL, NULL);
    M_ReadStringTable(
        file, g_GameFlow.num_titles, &g_GF_TitleFileNames,
        &g_GF_TitleFileNamesBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_fmvs, &g_GF_FMVFilenames, &g_GF_FMVFilenamesBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_LevelFileNames,
        &g_GF_LevelFileNamesBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_cutscenes, &g_GF_CutsceneFileNames,
        &g_GF_CutsceneFileNamesBuf);

    VFile_Read(
        file, &m_LevelOffsets, sizeof(int16_t) * (g_GameFlow.num_levels + 1));
    {
        const int16_t size = VFile_ReadS16(file);
        m_SequenceBuf = Memory_Alloc(size);
        VFile_Read(file, m_SequenceBuf, size);
    }

    m_FrontendSequence = m_SequenceBuf;
    for (int32_t i = 0; i < g_GameFlow.num_levels; i++) {
        m_ScriptTable[i] = m_SequenceBuf + (m_LevelOffsets[i + 1] / 2);
    }

    VFile_Read(file, g_GF_ValidDemos, sizeof(int16_t) * g_GameFlow.num_demos);

    // game strings
    if (VFile_ReadS16(file) != 89) {
        return false;
    }
    M_ReadStringTable(file, 89, NULL, NULL);

    M_ReadStringTable(file, 41, NULL, NULL);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Puzzle1Strings,
        &g_GF_Puzzle1StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Puzzle2Strings,
        &g_GF_Puzzle2StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Puzzle3Strings,
        &g_GF_Puzzle3StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Puzzle4Strings,
        &g_GF_Puzzle4StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Pickup1Strings,
        &g_GF_Pickup1StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Pickup2Strings,
        &g_GF_Pickup2StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Key1Strings, &g_GF_Key1StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Key2Strings, &g_GF_Key2StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Key3Strings, &g_GF_Key3StringsBuf);
    M_ReadStringTable(
        file, g_GameFlow.num_levels, &g_GF_Key4Strings, &g_GF_Key4StringsBuf);

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

    g_GameFlow.level_complete_track = MX_END_OF_LEVEL;
    return true;
}

bool GF_DoFrontendSequence(void)
{
    GF_N_LoadStrings(-1);
    const GAME_FLOW_COMMAND gf_cmd =
        GF_InterpretSequence(m_FrontendSequence, GFL_NORMAL);
    return gf_cmd.action == GF_EXIT_GAME;
}

GAME_FLOW_COMMAND GF_DoLevelSequence(
    const int32_t start_level, const GAME_FLOW_LEVEL_TYPE type)
{
    GF_N_LoadStrings(start_level);

    int32_t current_level = start_level;
    while (true) {
        if (current_level > g_GameFlow.num_levels - 1) {
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

    for (int32_t i = 0; i < GF_ADD_INV_NUMBER_OF; i++) {
        m_SecretInvItems[i] = 0;
        m_Add2InvItems[i] = 0;
    }

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
            if (ptr[1] > g_GameFlow.num_levels) {
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

                Music_Play(g_GameFlow.level_complete_track, MPM_ALWAYS);
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
                    m_SecretInvItems[ptr[1]]++;
                } else if (type != GFL_SAVED) {
                    m_Add2InvItems[ptr[1] - 1000]++;
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

void GF_ModifyInventory(const int32_t level, const int32_t type)
{
    START_INFO *const start = &g_SaveGame.start[level];

    if (!start->has_pistols && m_Add2InvItems[GF_ADD_INV_PISTOLS]) {
        start->has_pistols = 1;
        Inv_AddItem(O_PISTOL_ITEM);
    }

    M_ModifyInventory_GunOrAmmo(start, type, LGT_MAGNUMS);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_UZIS);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_SHOTGUN);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_HARPOON);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_M16);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_GRENADE);

    M_ModifyInventory_Item(type, O_FLARE_ITEM);
    M_ModifyInventory_Item(type, O_SMALL_MEDIPACK_ITEM);
    M_ModifyInventory_Item(type, O_LARGE_MEDIPACK_ITEM);
    M_ModifyInventory_Item(type, O_PICKUP_ITEM_1);
    M_ModifyInventory_Item(type, O_PICKUP_ITEM_2);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_1);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_2);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_3);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_4);
    M_ModifyInventory_Item(type, O_KEY_ITEM_1);
    M_ModifyInventory_Item(type, O_KEY_ITEM_2);
    M_ModifyInventory_Item(type, O_KEY_ITEM_3);
    M_ModifyInventory_Item(type, O_KEY_ITEM_4);

    for (int32_t i = 0; i < GF_ADD_INV_NUMBER_OF; i++) {
        if (type == 1) {
            m_SecretInvItems[i] = 0;
        } else if (type == 0) {
            m_Add2InvItems[i] = 0;
        }
    }
}

GAME_FLOW_COMMAND GF_StartDemo(const int32_t demo_num)
{
    const int32_t level_num = Demo_ChooseLevel(demo_num);
    if (level_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    return GF_DoLevelSequence(level_num, GFL_DEMO);
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

GAME_FLOW_COMMAND GF_EnterPhotoMode(void)
{
    PHASE *const subphase = Phase_PhotoMode_Create();
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
    Phase_PhotoMode_Destroy(subphase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_PauseGame(void)
{
    PHASE *const subphase = Phase_Pause_Create();
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
    Phase_Pause_Destroy(subphase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_ShowInventory(const INVENTORY_MODE mode)
{
    PHASE *const phase = Phase_Inventory_Create(mode);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Inventory_Destroy(phase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_ShowInventoryKeys(const GAME_OBJECT_ID receptacle_type_id)
{
    if (g_Config.gameplay.enable_auto_item_selection) {
        const GAME_OBJECT_ID object_id = Object_GetCognateInverse(
            receptacle_type_id, g_KeyItemToReceptacleMap);
        InvRing_SetRequestedObjectID(object_id);
    }
    return GF_ShowInventory(INV_KEYS_MODE);
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
