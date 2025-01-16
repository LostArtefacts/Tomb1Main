
#include "filesystem.h"
#include "game/game_string_table.h"
#include "game/game_string_table/priv.h"
#include "game/gameflow.h"
#include "json.h"
#include "log.h"
#include "memory.h"

#include <string.h>

static bool M_LoadTableFromJSON(
    JSON_OBJECT *root_obj, const char *key, GS_TABLE_ENTRY **dest);
static bool M_LoadLevelsFromJSON(JSON_OBJECT *obj, GS_FILE *gs_file);

static bool M_LoadTableFromJSON(
    JSON_OBJECT *const root_obj, const char *const key, GS_TABLE_ENTRY **dest)
{
    const JSON_OBJECT *const strings_obj = JSON_ObjectGetObject(root_obj, key);
    if (strings_obj == NULL) {
        // key is missing - rely on default strings
        return true;
    }

    *dest = Memory_Alloc(sizeof(GS_TABLE_ENTRY) * (strings_obj->length + 1));

    GS_TABLE_ENTRY *cur = *dest;
    JSON_OBJECT_ELEMENT *strings_elem = strings_obj->start;
    for (size_t i = 0; i < strings_obj->length;
         i++, strings_elem = strings_elem->next) {
        const char *const key = strings_elem->name->string;
        const char *const value = JSON_ObjectGetString(strings_obj, key, NULL);
        if (value == NULL) {
            LOG_ERROR("invalid string key %s", strings_elem->name->string);
            return NULL;
        }
        cur->key = Memory_DupStr(key);
        cur->value = Memory_DupStr(value);
        cur++;
    }

    cur->key = NULL;
    cur->value = NULL;
    return true;
}

static bool M_LoadLevelsFromJSON(JSON_OBJECT *const obj, GS_FILE *const gs_file)
{
    bool result = true;

    JSON_ARRAY *const jlvl_arr = JSON_ObjectGetArray(obj, "levels");
    if (jlvl_arr == NULL) {
        LOG_ERROR("'levels' must be a list");
        result = false;
        goto end;
    }

    int32_t level_count = jlvl_arr->length;
    if (level_count != GameFlow_GetLevelCount()) {
        LOG_ERROR(
            "'levels' must have exactly %d levels, as we still rely on legacy "
            "tombpc.dat",
            GameFlow_GetLevelCount());
        result = false;
        goto end;
    }

    gs_file->level_count = level_count;
    gs_file->levels = Memory_Alloc(sizeof(GS_TABLE) * level_count);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    for (size_t i = 0; i < jlvl_arr->length; i++, jlvl_elem = jlvl_elem->next) {
        GS_TABLE *const level = &gs_file->levels[i];

        JSON_OBJECT *const jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (jlvl_obj == NULL) {
            LOG_ERROR("'levels' elements must be dictionaries");
            result = false;
            goto end;
        }

        result &=
            M_LoadTableFromJSON(jlvl_obj, "object_names", &level->object_names);
        result &= M_LoadTableFromJSON(
            jlvl_obj, "object_descriptions", &level->object_descriptions);
        result &=
            M_LoadTableFromJSON(jlvl_obj, "game_strings", &level->game_strings);
    }

end:
    return result;
}

bool GameStringTable_LoadFromFile(const char *const path)
{
    GameStringTable_Shutdown();

    bool result = true;
    JSON_VALUE *root = NULL;

    char *script_data = NULL;
    if (!File_Load(path, &script_data, NULL)) {
        LOG_ERROR("failed to open strings file");
        result = false;
        goto end;
    }

    JSON_PARSE_RESULT parse_result;
    root = JSON_ParseEx(
        script_data, strlen(script_data), JSON_PARSE_FLAGS_ALLOW_JSON5, NULL,
        NULL, &parse_result);
    if (root == NULL) {
        LOG_ERROR(
            "failed to parse script file: %s in line %d, char %d",
            JSON_GetErrorDescription(parse_result.error),
            parse_result.error_line_no, parse_result.error_row_no, script_data);
        result = false;
        goto end;
    }

    GS_FILE *const gs_file = &g_GST_File;
    JSON_OBJECT *root_obj = JSON_ValueAsObject(root);
    result &= M_LoadTableFromJSON(
        root_obj, "object_names", &gs_file->global.object_names);
    result &= M_LoadTableFromJSON(
        root_obj, "object_descriptions", &gs_file->global.object_descriptions);
    result &= M_LoadTableFromJSON(
        root_obj, "game_strings", &gs_file->global.game_strings);
    result &= M_LoadLevelsFromJSON(root_obj, gs_file);

end:
    if (root != NULL) {
        JSON_ValueFree(root);
        root = NULL;
    }

    if (!result) {
        GameStringTable_Shutdown();
    }

    Memory_FreePointer(&script_data);
    return result;
}
