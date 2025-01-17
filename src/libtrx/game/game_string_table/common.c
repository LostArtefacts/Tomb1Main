#include "debug.h"
#include "game/game_string.h"
#include "game/game_string_table.h"
#include "game/game_string_table/priv.h"
#include "game/gameflow.h"
#include "game/objects/names.h"
#include "log.h"
#include "memory.h"

#include <string.h>

typedef void (*M_LOAD_STRING_FUNC)(const char *, const char *);

GS_FILE g_GST_File = {};

static struct {
    GAME_OBJECT_ID object_id;
    const char *key_name;
} m_ObjectKeyNames[] = {
#define OBJ_ALIAS_DEFINE(object_id_, source_object_id_)
#define OBJ_NAME_DEFINE(object_id_, key_name_, default_name)                   \
    { .object_id = object_id_, .key_name = key_name_ },
#include "game/objects/names.def"
#undef OBJ_ALIAS_DEFINE
#undef OBJ_NAME_DEFINE
    { .object_id = NO_OBJECT },
};

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

static GAME_OBJECT_ID M_GetObjectID(const char *const key);
static void M_Apply(const GS_TABLE *table);

static GAME_OBJECT_ID M_GetObjectID(const char *const key)
{
    for (int32_t i = 0; m_ObjectKeyNames[i].object_id != NO_OBJECT; i++) {
        if (strcmp(m_ObjectKeyNames[i].key_name, key) == 0) {
            return m_ObjectKeyNames[i].object_id;
        }
    }
    return NO_OBJECT;
}

static void M_DoObjectAliases(void)
{
    for (int32_t i = 0; m_ObjectAliases[i].target_object_id != NO_OBJECT; i++) {
        const GAME_OBJECT_ID target_object_id =
            m_ObjectAliases[i].target_object_id;
        const GAME_OBJECT_ID source_object_id =
            m_ObjectAliases[i].source_object_id;
        Object_SetName(target_object_id, Object_GetName(source_object_id));
        const char *const description = Object_GetDescription(source_object_id);
        if (description != NULL) {
            Object_SetDescription(target_object_id, description);
        }
    }
}

static void M_Apply(const GS_TABLE *const table)
{
    {
        const GS_GAME_STRING_ENTRY *cur = table->game_strings;
        while (cur != NULL && cur->key != NULL) {
            if (!GameString_IsKnown(cur->key)) {
                LOG_ERROR("Invalid game string key: %s", cur->key);
            } else if (cur->value == NULL) {
                LOG_ERROR("Invalid game string value: %s", cur->key);
            } else {
                GameString_Define(cur->key, cur->value);
            }
            cur++;
        }
    }

    {
        const GS_OBJECT_ENTRY *cur = table->objects;
        while (cur != NULL && cur->key != NULL) {
            const GAME_OBJECT_ID object_id = M_GetObjectID(cur->key);
            if (object_id == NO_OBJECT) {
                LOG_ERROR("Invalid object id: %s", cur->key);
            } else {
                if (cur->name == NULL) {
                    LOG_ERROR("Invalid object name: %s", cur->key);
                } else {
                    Object_SetName(object_id, cur->name);
                }
                if (cur->description != NULL) {
                    Object_SetDescription(object_id, cur->description);
                }
            }
            cur++;
        }
    }
}

void GameStringTable_Apply(const int32_t level_num)
{
    const GS_FILE *const gs_file = &g_GST_File;

    if (level_num < -1 || level_num >= gs_file->level_count) {
        LOG_WARNING(
            "Trying to apply unavailable strings for level %d", level_num);
        return;
    }

    LOG_DEBUG("loading file %d", level_num);
    Object_ResetNames();
    M_Apply(&gs_file->global);
    for (int32_t i = 0; i < GameFlow_GetLevelCount(); i++) {
        GameFlow_SetLevelTitle(i, gs_file->levels[i].title);
    }
    if (level_num != -1) {
        M_Apply(&gs_file->levels[level_num].table);
    }
    M_DoObjectAliases();
}

void GameStringTable_Shutdown(void)
{
    GS_File_Free(&g_GST_File);
}
