#include "debug.h"
#include "enum_map.h"
#include "game/game_string.h"
#include "game/game_string_table.h"
#include "game/game_string_table/priv.h"
#include "game/objects/names.h"
#include "log.h"
#include "memory.h"

#include <stddef.h>

typedef void (*M_LOAD_STRING_FUNC)(const char *, const char *);

GS_FILE g_GST_File = {};

static struct {
    GAME_OBJECT_ID object_id;
    GAME_STRING_ID game_string_id;
} m_ObjectAliases[] = {
#define OBJ_ALIAS_DEFINE(object_id_, game_string_id_)                          \
    { .object_id = object_id_, .game_string_id = game_string_id_ },
#include "./aliases.def"
#undef OBJ_ALIAS_DEFINE
    { .object_id = NO_OBJECT },
};

static void M_GameStringTableEntry_Free(GS_TABLE_ENTRY *entry);
static void M_GameStringTable_Free(GS_TABLE *gs_table);
static void M_GameStringFile_Free(GS_FILE *gs_file);
static void M_ApplyGameString(const char *key, const char *value);
static void M_ApplyObjectString(const char *key, const char *value);
static void M_ApplyEntries(
    const GS_TABLE_ENTRY *const entries, M_LOAD_STRING_FUNC load_func);
static void M_ApplyStrings(
    const GS_TABLE_ENTRY *global_entries, const GS_TABLE_ENTRY *level_entries,
    M_LOAD_STRING_FUNC load_func);
static void M_ApplyGameStrings(const GS_FILE *gs_table, int32_t level_num);
static void M_ApplyObjectStrings(const GS_FILE *gs_table, int32_t level_num);

static void M_GameStringTableEntry_Free(GS_TABLE_ENTRY *const entry)
{
    if (entry == NULL) {
        return;
    }
    GS_TABLE_ENTRY *cur = entry;
    while (cur->key != NULL) {
        Memory_FreePointer(&cur->key);
        Memory_FreePointer(&cur->value);
        cur++;
    }
    Memory_Free(entry);
}

static void M_GameStringTable_Free(GS_TABLE *const gs_table)
{
    M_GameStringTableEntry_Free(gs_table->object_strings);
    gs_table->object_strings = NULL;
    M_GameStringTableEntry_Free(gs_table->game_strings);
    gs_table->game_strings = NULL;
}

static void M_GameStringFile_Free(GS_FILE *const gs_file)
{
    M_GameStringTable_Free(&gs_file->global);
    for (int32_t i = 0; i < gs_file->level_count; i++) {
        M_GameStringTable_Free(&gs_file->levels[i]);
    }
    Memory_FreePointer(&gs_file->levels);
    gs_file->level_count = 0;
}

static void M_ApplyGameString(const char *const key, const char *const value)
{
    if (!GameString_IsKnown(key)) {
        LOG_ERROR("Invalid game string key: %s", key);
    } else if (value == NULL) {
        LOG_ERROR("Invalid game string value: %s", key);
    } else {
        GameString_Define(key, value);
    }
}

static void M_ApplyObjectString(const char *const key, const char *const value)
{
    const GAME_OBJECT_ID object_id =
        ENUM_MAP_GET(GAME_OBJECT_ID, key, NO_OBJECT);
    if (object_id == NO_OBJECT) {
        LOG_ERROR("Invalid object id: %s", key);
    } else {
        Object_SetName(object_id, value);
    }
}

static void M_ApplyEntries(
    const GS_TABLE_ENTRY *entries, const M_LOAD_STRING_FUNC load_func)
{
    while (entries != NULL && entries->key != NULL) {
        load_func(entries->key, entries->value);
        entries++;
    }
}

static void M_ApplyStrings(
    const GS_TABLE_ENTRY *const global_entries,
    const GS_TABLE_ENTRY *const level_entries,
    const M_LOAD_STRING_FUNC load_func)
{
    M_ApplyEntries(global_entries, load_func);
    if (level_entries != NULL) {
        M_ApplyEntries(level_entries, load_func);
    }
}

static void M_ApplyGameStrings(
    const GS_FILE *const gs_file, const int32_t level_num)
{
    ASSERT(level_num >= -1 && level_num < gs_file->level_count);
    M_ApplyStrings(
        gs_file->global.game_strings,
        level_num != -1 ? gs_file->levels[level_num].game_strings : NULL,
        M_ApplyGameString);
}

static void M_ApplyObjectStrings(
    const GS_FILE *const gs_file, const int32_t level_num)
{
    ASSERT(level_num >= -1 && level_num < gs_file->level_count);
    M_ApplyStrings(
        gs_file->global.object_strings,
        level_num != -1 ? gs_file->levels[level_num].object_strings : NULL,
        M_ApplyObjectString);
}

void GameStringTable_Apply(const int32_t level_num)
{
    Object_ResetNames();

    const GS_FILE *const gs_file = &g_GST_File;
    M_ApplyObjectStrings(gs_file, level_num);
    M_ApplyGameStrings(gs_file, level_num);

    for (int32_t i = 0; m_ObjectAliases[i].object_id != NO_OBJECT; i++) {
        const char *const new_name =
            GameString_Get(m_ObjectAliases[i].game_string_id);
        if (new_name != NULL) {
            Object_SetName(m_ObjectAliases[i].object_id, new_name);
        }
    }
}

void GameStringTable_Shutdown(void)
{
    M_GameStringFile_Free(&g_GST_File);
}
