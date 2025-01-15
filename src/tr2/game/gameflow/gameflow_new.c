#include "game/gameflow/gameflow_new.h"

#include "game/game_string.h"
#include "game/requester.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/enum_map.h>
#include <libtrx/game/gameflow/types.h>
#include <libtrx/game/objects/names.h>
#include <libtrx/log.h>

GAME_FLOW_NEW g_GameFlowNew;
GAME_INFO g_GameInfo;

static void M_LoadObjectString(const char *key, const char *value);
static void M_LoadGameString(const char *key, const char *value);
static void M_LoadObjectStrings(const int32_t level_num);
static void M_LoadGameStrings(const int32_t level_num);

static void M_LoadObjectString(const char *const key, const char *const value)
{
    const GAME_OBJECT_ID object_id =
        ENUM_MAP_GET(GAME_OBJECT_ID, key, NO_OBJECT);
    if (object_id == NO_OBJECT) {
        LOG_ERROR("Invalid object id: %s", key);
    } else {
        Object_SetName(object_id, value);
    }
}

static void M_LoadGameString(const char *const key, const char *const value)
{
    if (!GameString_IsKnown(key)) {
        LOG_ERROR("Invalid game string key: %s", key);
    } else if (value == NULL) {
        LOG_ERROR("Invalid game string value: %s", key);
    } else {
        GameString_Define(key, value);
    }
}

static void M_LoadObjectStrings(const int32_t level_num)
{
    const GAME_FLOW_NEW *const gf = &g_GameFlowNew;

    const GAME_FLOW_NEW_STRING_ENTRY *entry = gf->object_strings;
    while (entry != NULL && entry->key != NULL) {
        M_LoadObjectString(entry->key, entry->value);
        entry++;
    }

    if (level_num >= 0) {
        ASSERT(level_num < gf->level_count);
        const GAME_FLOW_NEW_LEVEL *const level = &gf->levels[level_num];
        entry = level->object_strings;
        while (entry != NULL && entry->key != NULL) {
            M_LoadObjectString(entry->key, entry->value);
            entry++;
        }
    }
}

static void M_LoadGameStrings(const int32_t level_num)
{
    const GAME_FLOW_NEW *const gf = &g_GameFlowNew;

    const GAME_FLOW_NEW_STRING_ENTRY *entry = gf->game_strings;
    while (entry != NULL && entry->key != NULL) {
        M_LoadGameString(entry->key, entry->value);
        entry++;
    }

    if (level_num >= 0) {
        ASSERT(level_num < gf->level_count);
        const GAME_FLOW_NEW_LEVEL *const level = &gf->levels[level_num];
        entry = level->game_strings;
        while (entry != NULL && entry->key != NULL) {
            M_LoadGameString(entry->key, entry->value);
            entry++;
        }
    }
}

void GF_N_LoadStrings(const int32_t level_num)
{
    Object_ResetNames();

    M_LoadObjectStrings(level_num);
    M_LoadGameStrings(level_num);

    struct {
        GAME_OBJECT_ID object_id;
        const char *name;
    } game_string_defs[] = {
        // clang-format off
        { O_COMPASS_OPTION,        GS(INV_ITEM_STOPWATCH) },
        { O_COMPASS_ITEM,          GS(INV_ITEM_STOPWATCH) },
        { O_PISTOL_ITEM,           GS(INV_ITEM_PISTOLS) },
        { O_PISTOL_OPTION,         GS(INV_ITEM_PISTOLS) },
        { O_FLARE_ITEM,            GS(INV_ITEM_FLARE) },
        { O_FLARES_OPTION,         GS(INV_ITEM_FLARE) },
        { O_SHOTGUN_ITEM,          GS(INV_ITEM_SHOTGUN) },
        { O_SHOTGUN_OPTION,        GS(INV_ITEM_SHOTGUN) },
        { O_MAGNUM_ITEM,           GS(INV_ITEM_MAGNUMS) },
        { O_MAGNUM_OPTION,         GS(INV_ITEM_MAGNUMS) },
        { O_UZI_ITEM,              GS(INV_ITEM_UZIS) },
        { O_UZI_OPTION,            GS(INV_ITEM_UZIS) },
        { O_HARPOON_ITEM,          GS(INV_ITEM_HARPOON) },
        { O_HARPOON_OPTION,        GS(INV_ITEM_HARPOON) },
        { O_M16_ITEM,              GS(INV_ITEM_M16) },
        { O_M16_OPTION,            GS(INV_ITEM_M16) },
        { O_GRENADE_ITEM,          GS(INV_ITEM_GRENADE) },
        { O_GRENADE_OPTION,        GS(INV_ITEM_GRENADE) },
        { O_PISTOL_AMMO_ITEM,      GS(INV_ITEM_PISTOL_AMMO) },
        { O_PISTOL_AMMO_OPTION,    GS(INV_ITEM_PISTOL_AMMO) },
        { O_SHOTGUN_AMMO_ITEM,     GS(INV_ITEM_SHOTGUN_AMMO) },
        { O_SHOTGUN_AMMO_OPTION,   GS(INV_ITEM_SHOTGUN_AMMO) },
        { O_MAGNUM_AMMO_ITEM,      GS(INV_ITEM_MAGNUM_AMMO) },
        { O_MAGNUM_AMMO_OPTION,    GS(INV_ITEM_MAGNUM_AMMO) },
        { O_UZI_AMMO_ITEM,         GS(INV_ITEM_UZI_AMMO) },
        { O_UZI_AMMO_OPTION,       GS(INV_ITEM_UZI_AMMO) },
        { O_HARPOON_AMMO_ITEM,     GS(INV_ITEM_HARPOON_AMMO) },
        { O_HARPOON_AMMO_OPTION,   GS(INV_ITEM_HARPOON_AMMO) },
        { O_M16_AMMO_ITEM,         GS(INV_ITEM_M16_AMMO) },
        { O_M16_AMMO_OPTION,       GS(INV_ITEM_M16_AMMO) },
        { O_GRENADE_AMMO_ITEM,     GS(INV_ITEM_GRENADE_AMMO) },
        { O_GRENADE_AMMO_OPTION,   GS(INV_ITEM_GRENADE_AMMO) },
        { O_SMALL_MEDIPACK_ITEM,   GS(INV_ITEM_SMALL_MEDIPACK) },
        { O_SMALL_MEDIPACK_OPTION, GS(INV_ITEM_SMALL_MEDIPACK) },
        { O_LARGE_MEDIPACK_ITEM,   GS(INV_ITEM_LARGE_MEDIPACK) },
        { O_LARGE_MEDIPACK_OPTION, GS(INV_ITEM_LARGE_MEDIPACK) },
        { O_PICKUP_ITEM_1,         GS(INV_ITEM_PICKUP_1) },
        { O_PICKUP_OPTION_1,       GS(INV_ITEM_PICKUP_1) },
        { O_PICKUP_ITEM_2,         GS(INV_ITEM_PICKUP_2) },
        { O_PICKUP_OPTION_2,       GS(INV_ITEM_PICKUP_2) },
        { O_PUZZLE_ITEM_1,         GS(INV_ITEM_PUZZLE_1) },
        { O_PUZZLE_OPTION_1,       GS(INV_ITEM_PUZZLE_1) },
        { O_PUZZLE_ITEM_2,         GS(INV_ITEM_PUZZLE_2) },
        { O_PUZZLE_OPTION_2,       GS(INV_ITEM_PUZZLE_2) },
        { O_PUZZLE_ITEM_3,         GS(INV_ITEM_PUZZLE_3) },
        { O_PUZZLE_OPTION_3,       GS(INV_ITEM_PUZZLE_3) },
        { O_PUZZLE_ITEM_4,         GS(INV_ITEM_PUZZLE_4) },
        { O_PUZZLE_OPTION_4,       GS(INV_ITEM_PUZZLE_4) },
        { O_KEY_ITEM_1,            GS(INV_ITEM_KEY_1) },
        { O_KEY_OPTION_1,          GS(INV_ITEM_KEY_1) },
        { O_KEY_ITEM_2,            GS(INV_ITEM_KEY_2) },
        { O_KEY_OPTION_2,          GS(INV_ITEM_KEY_2) },
        { O_KEY_ITEM_3,            GS(INV_ITEM_KEY_3) },
        { O_KEY_OPTION_3,          GS(INV_ITEM_KEY_3) },
        { O_KEY_ITEM_4,            GS(INV_ITEM_KEY_4) },
        { O_KEY_OPTION_4,          GS(INV_ITEM_KEY_4) },
        { O_PASSPORT_OPTION,       GS(INV_ITEM_GAME) },
        { O_PASSPORT_CLOSED,       GS(INV_ITEM_GAME) },
        { O_DETAIL_OPTION,         GS(INV_ITEM_DETAILS) },
        { O_SOUND_OPTION,          GS(INV_ITEM_SOUND) },
        { O_CONTROL_OPTION,        GS(INV_ITEM_CONTROLS) },
        { O_PHOTO_OPTION,          GS(INV_ITEM_LARAS_HOME) },
        { NO_OBJECT,               NULL },
        // clang-format on
    };

    for (int32_t i = 0; game_string_defs[i].object_id != NO_OBJECT; i++) {
        const char *const new_name = game_string_defs[i].name;
        if (new_name != NULL) {
            Object_SetName(game_string_defs[i].object_id, new_name);
        }
    }

    Requester_SetHeading(
        &g_LoadGameRequester, GS(PASSPORT_SELECT_LEVEL), 0, 0, 0);
    Requester_SetHeading(
        &g_SaveGameRequester, GS(PASSPORT_SELECT_LEVEL), 0, 0, 0);
}

int32_t GameFlow_GetLevelCount(void)
{
    return g_GameFlowNew.level_count;
}

int32_t GameFlow_GetDemoCount(void)
{
    return g_GameFlow.num_demos;
}

const char *GameFlow_GetLevelFileName(int32_t level_num)
{
    return g_GF_LevelFileNames[level_num];
}

const char *GameFlow_GetLevelTitle(int32_t level_num)
{
    return g_GF_LevelNames[level_num];
}

int32_t GameFlow_GetGymLevelNum(void)
{
    return g_GameFlow.gym_enabled ? LV_GYM : -1;
}

void GameFlow_OverrideCommand(const GAME_FLOW_COMMAND command)
{
    g_GF_OverrideCommand = command;
}

GAME_FLOW_COMMAND GameFlow_GetOverrideCommand(void)
{
    return g_GF_OverrideCommand;
}
