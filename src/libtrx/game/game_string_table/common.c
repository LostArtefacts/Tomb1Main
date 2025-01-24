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
            const GAME_OBJECT_ID object_id = Object_IdFromKey(cur->key);
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

void GameStringTable_Apply(const GAME_FLOW_LEVEL *const level)
{
    const GS_FILE *const gs_file = &g_GST_File;

    Object_ResetNames();
    M_Apply(&gs_file->global);
    for (int32_t i = 0; i < GF_GetLevelCount(GFL_NORMAL); i++) {
        GF_SetLevelTitle(
            GF_GetLevel(i, GFL_NORMAL), gs_file->levels.entries[i].title);
    }

#if TR_VERSION == 2
    // TODO: TR1 still has everything in a single linear sequence
    for (int32_t i = 0; i < GF_GetLevelCount(GFL_DEMO); i++) {
        GF_SetLevelTitle(
            GF_GetLevel(i, GFL_DEMO), gs_file->demos.entries[i].title);
    }
    for (int32_t i = 0; i < GF_GetLevelCount(GFL_CUTSCENE); i++) {
        GF_SetLevelTitle(
            GF_GetLevel(i, GFL_CUTSCENE), gs_file->cutscenes.entries[i].title);
    }
#endif

    if (level != NULL) {
#if TR_VERSION == 1
        // TODO: TR1 still has everything in a single linear sequence
        const GS_LEVEL_TABLE *const level_table = &gs_file->levels;
#elif TR_VERSION == 2
        const GS_LEVEL_TABLE *level_table = NULL;
        switch (level->type) {
        case GFL_NORMAL:
        case GFL_SAVED:
            level_table = &gs_file->levels;
            break;
        case GFL_DEMO:
            level_table = &gs_file->demos;
            break;
        case GFL_CUTSCENE:
            level_table = &gs_file->cutscenes;
            break;
        case GFL_TITLE:
            level_table = NULL;
            break;
        default:
            ASSERT_FAIL();
        }
#endif

        if (level_table != NULL) {
            ASSERT(level->num >= 0);
            ASSERT(level->num < level_table->count);
            M_Apply(&level_table->entries[level->num].table);
        }
    }
    M_DoObjectAliases();
}

void GameStringTable_Shutdown(void)
{
    GS_File_Free(&g_GST_File);
}
