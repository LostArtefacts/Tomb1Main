#include "game/game_flow/common.h"

#include "debug.h"
#include "game/game_flow/vars.h"
#include "memory.h"

static const GAME_FLOW_LEVEL *m_CurrentLevel = NULL;
static GAME_FLOW_COMMAND m_OverrideCommand = { .action = GF_NOOP };

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);
static void M_FreeInjections(INJECTION_DATA *injections);
static void M_FreeLevel(GAME_FLOW_LEVEL *level);
static void M_FreeLevelTable(GAME_FLOW_LEVEL_TABLE *level_table);
static void M_FreeFMVs(GAME_FLOW *gf);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    Memory_Free(sequence->events);
}

static void M_FreeInjections(INJECTION_DATA *const injections)
{
    for (int32_t i = 0; i < injections->count; i++) {
        Memory_FreePointer(&injections->data_paths[i]);
    }
    Memory_FreePointer(&injections->data_paths);
}

static void M_FreeLevel(GAME_FLOW_LEVEL *const level)
{
    Memory_FreePointer(&level->path);
    Memory_FreePointer(&level->title);
    M_FreeInjections(&level->injections);
    M_FreeSequence(&level->sequence);

#if TR_VERSION == 1
    if (level->item_drops.count > 0) {
        for (int32_t i = 0; i < level->item_drops.count; i++) {
            Memory_FreePointer(&level->item_drops.data[i].object_ids);
        }
        Memory_FreePointer(&level->item_drops.data);
    }
#endif
}

static void M_FreeLevelTable(GAME_FLOW_LEVEL_TABLE *const level_table)
{
    if (level_table != NULL) {
        for (int32_t i = 0; i < level_table->count; i++) {
            M_FreeLevel(&level_table->levels[i]);
        }
        Memory_FreePointer(&level_table->levels);
        level_table->count = 0;
    }
}

static void M_FreeFMVs(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->fmv_count; i++) {
        Memory_FreePointer(&gf->fmvs[i].path);
    }
    Memory_FreePointer(&gf->fmvs);
    gf->fmv_count = 0;
}

void GF_Shutdown(void)
{
    GAME_FLOW *const gf = &g_GameFlow;
    M_FreeInjections(&gf->injections);

    for (int32_t i = 0; i < GFLT_NUMBER_OF; i++) {
        M_FreeLevelTable(&gf->level_tables[i]);
    }
    M_FreeFMVs(gf);

    if (gf->title_level != NULL) {
        M_FreeLevel(gf->title_level);
        Memory_FreePointer(&gf->title_level);
    }

#if TR_VERSION == 1
    Memory_FreePointer(&gf->main_menu_background_path);
    Memory_FreePointer(&gf->savegame_fmt_legacy);
    Memory_FreePointer(&gf->savegame_fmt_bson);
#endif
}

void GF_OverrideCommand(const GAME_FLOW_COMMAND command)
{
    m_OverrideCommand = command;
}

GAME_FLOW_COMMAND GF_GetOverrideCommand(void)
{
    return m_OverrideCommand;
}

GAME_FLOW_LEVEL_TABLE_TYPE GF_GetLevelTableType(
    const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_GYM:
    case GFL_NORMAL:
    case GFL_BONUS:
#if TR_VERSION == 1
    case GFL_DUMMY:
    case GFL_CURRENT:
#endif
        return GFLT_MAIN;

    case GFL_CUTSCENE:
        return GFLT_CUTSCENES;

    case GFL_DEMO:
        return GFLT_DEMOS;

    case GFL_TITLE:
        // this needs to be handled separately
        return GFLT_UNKNOWN;
    }

    ASSERT_FAIL();
}

const GAME_FLOW_LEVEL_TABLE *GF_GetLevelTable(
    const GAME_FLOW_LEVEL_TABLE_TYPE level_table_type)
{
    return &g_GameFlow.level_tables[level_table_type];
}

const GAME_FLOW_LEVEL *GF_GetCurrentLevel(void)
{
    return m_CurrentLevel;
}

const GAME_FLOW_LEVEL *GF_GetTitleLevel(void)
{
    return g_GameFlow.title_level;
}

const GAME_FLOW_LEVEL *GF_GetGymLevel(void)
{
    const GAME_FLOW_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(GFLT_MAIN);
    for (int32_t i = 0; i < level_table->count; i++) {
        const GAME_FLOW_LEVEL *const level = &level_table->levels[i];
        if (level->type == GFL_GYM) {
            return level;
        }
    }
    return NULL;
}

const GAME_FLOW_LEVEL *GF_GetFirstLevel(void)
{
    const GAME_FLOW_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(GFLT_MAIN);
    for (int32_t i = 0; i < level_table->count; i++) {
        const GAME_FLOW_LEVEL *const level = &level_table->levels[i];
        if (level->type == GFL_GYM) {
            continue;
        }
        return level;
    }
    return NULL;
}

const GAME_FLOW_LEVEL *GF_GetLastLevel(void)
{
    const GAME_FLOW_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(GFLT_MAIN);
    const GAME_FLOW_LEVEL *result = NULL;
    for (int32_t i = 0; i < level_table->count; i++) {
        const GAME_FLOW_LEVEL *const level = &level_table->levels[i];
        if (level->type == GFL_GYM) {
            continue;
        }
#if TR_VERSION == 1
        if (level->type == GFL_DUMMY || level->type == GFL_CURRENT) {
            continue;
        }
#endif
        result = level;
    }
    return result;
}

const GAME_FLOW_LEVEL *GF_GetLevel(
    const GAME_FLOW_LEVEL_TABLE_TYPE level_table_type, const int32_t num)
{
    const GAME_FLOW_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(level_table_type);
    ASSERT(level_table != NULL);
    if (num < 0 || num >= level_table->count) {
        LOG_ERROR("Invalid cutscene number: %d", num);
        return NULL;
    }
    return &level_table->levels[num];
}

const GAME_FLOW_LEVEL *GF_GetLevelAfter(const GAME_FLOW_LEVEL *const level)
{
    const GAME_FLOW_LEVEL_TABLE_TYPE level_table_type =
        GF_GetLevelTableType(level->type);
    const GAME_FLOW_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(level_table_type);
    return level->num + 1 < level_table->count
        ? &level_table->levels[level->num + 1]
        : NULL;
}

const GAME_FLOW_LEVEL *GF_GetLevelBefore(const GAME_FLOW_LEVEL *const level)
{
    const GAME_FLOW_LEVEL_TABLE_TYPE level_table_type =
        GF_GetLevelTableType(level->type);
    const GAME_FLOW_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(level_table_type);
    return level->num - 1 >= 0 ? &level_table->levels[level->num - 1] : NULL;
}

void GF_SetCurrentLevel(const GAME_FLOW_LEVEL *const level)
{
    m_CurrentLevel = level;
}

void GF_SetLevelTitle(GAME_FLOW_LEVEL *const level, const char *const title)
{
    Memory_FreePointer(&level->title);
    level->title = title != NULL ? Memory_DupStr(title) : NULL;
}
