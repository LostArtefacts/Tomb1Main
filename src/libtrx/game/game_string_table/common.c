#include "debug.h"
#include "game/game_flow.h"
#include "game/game_string.h"
#include "game/game_string_table.h"
#include "game/game_string_table/priv.h"
#include "game/objects/names.h"
#include "log.h"
#include "memory.h"

#include <string.h>

typedef void (*M_LOAD_STRING_FUNC)(const char *, const char *);

GS_FILE g_GST_File = {};

static struct {
    GAME_OBJECT_ID target_object_id;
    GAME_OBJECT_ID source_object_id;
} m_ObjectAliases[] = {
#define OBJ_ALIAS_DEFINE(target_object_id_, source_object_id_)                 \
    { .target_object_id = target_object_id_,                                   \
      .source_object_id = source_object_id_ },
#define OBJ_NAME_DEFINE(object_id_, key_name_, default_name)
#include "game/objects/names.def"
#undef OBJ_ALIAS_DEFINE
#undef OBJ_NAME_DEFINE
    { .target_object_id = NO_OBJECT },
};

static void M_Apply(const GS_TABLE *table);
static void M_ApplyLevelTitles(
    const GS_FILE *gs_file, GF_LEVEL_TABLE_TYPE level_table_type);

static void M_DoObjectAliases(void)
{
    for (int32_t i = 0; m_ObjectAliases[i].target_object_id != NO_OBJECT; i++) {
        const GAME_OBJECT_ID target_object_id =
            m_ObjectAliases[i].target_object_id;
        const GAME_OBJECT_ID source_object_id =
            m_ObjectAliases[i].source_object_id;
        Object_SetName(target_object_id, Object_GetName(source_object_id));
        const char *const description = Object_GetDescription(source_object_id);
        if (description != nullptr) {
            Object_SetDescription(target_object_id, description);
        }
    }
}

static void M_Apply(const GS_TABLE *const table)
{
    {
        const GS_GAME_STRING_ENTRY *cur = table->game_strings;
        while (cur != nullptr && cur->key != nullptr) {
            if (!GameString_IsKnown(cur->key)) {
                LOG_ERROR("Invalid game string key: %s", cur->key);
            } else if (cur->value == nullptr) {
                LOG_ERROR("Invalid game string value: %s", cur->key);
            } else {
                GameString_Define(cur->key, cur->value);
            }
            cur++;
        }
    }

    {
        const GS_OBJECT_ENTRY *cur = table->objects;
        while (cur != nullptr && cur->key != nullptr) {
            const GAME_OBJECT_ID obj_id = Object_IdFromKey(cur->key);
            if (obj_id == NO_OBJECT) {
                LOG_ERROR("Invalid object id: %s", cur->key);
            } else {
                if (cur->name == nullptr) {
                    LOG_ERROR("Invalid object name: %s", cur->key);
                } else {
                    Object_SetName(obj_id, cur->name);
                }
                if (cur->description != nullptr) {
                    Object_SetDescription(obj_id, cur->description);
                }
            }
            cur++;
        }
    }
}

static void M_ApplyLevelTitles(
    const GS_FILE *const gs_file, const GF_LEVEL_TABLE_TYPE level_table_type)
{
    const GF_LEVEL_TABLE *const level_table =
        GF_GetLevelTable(level_table_type);
    const GS_LEVEL_TABLE *const gs_level_table =
        &gs_file->level_tables[level_table_type];
    for (int32_t i = 0; i < level_table->count; i++) {
        GF_SetLevelTitle(
            &level_table->levels[i], gs_level_table->entries[i].title);
    }
}

void GameStringTable_Apply(const GF_LEVEL *const level)
{
    const GS_FILE *const gs_file = &g_GST_File;

    Object_ResetNames();
    M_Apply(&gs_file->global);

    for (int32_t i = 0; i < GFLT_NUMBER_OF; i++) {
        M_ApplyLevelTitles(gs_file, GFLT_MAIN);
    }

    if (level != nullptr) {
        const GS_LEVEL_TABLE *gs_level_table = nullptr;
        switch (level->type) {
        case GFL_TITLE:
            gs_level_table = nullptr;
            break;
        default: {
            const GF_LEVEL_TABLE_TYPE level_table_type =
                GF_GetLevelTableType(level->type);
            gs_level_table = &gs_file->level_tables[level_table_type];
        }
        }

        if (gs_level_table != nullptr) {
            ASSERT(level->num >= 0);
            ASSERT(level->num < gs_level_table->count);
            M_Apply(&gs_level_table->entries[level->num].table);
        }
    }
    M_DoObjectAliases();
}

void GameStringTable_Shutdown(void)
{
    GS_File_Free(&g_GST_File);
}
