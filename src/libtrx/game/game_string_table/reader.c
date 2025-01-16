#include "filesystem.h"
#include "game/game_string_table.h"
#include "game/game_string_table/priv.h"
#include "json.h"
#include "log.h"
#include "memory.h"

#include <string.h>

static bool M_LoadTableFromJSON(JSON_OBJECT *root_obj, GS_TABLE *out_table);
static bool M_LoadLevelsFromJSON(JSON_OBJECT *obj, GS_FILE *gs_file);

static bool M_LoadTableFromJSON(
    JSON_OBJECT *const root_obj, GS_TABLE *const out_table)
{
    bool result = true;

    // Load objects
    JSON_OBJECT *const jobjs = JSON_ObjectGetObject(root_obj, "objects");
    if (jobjs != NULL) {
        const size_t object_count = jobjs->length;
        out_table->objects =
            Memory_Alloc(sizeof(GS_OBJECT_ENTRY) * (object_count + 1));

        JSON_OBJECT_ELEMENT *jobj_elem = jobjs->start;
        for (size_t i = 0; i < object_count; i++, jobj_elem = jobj_elem->next) {
            JSON_OBJECT *const jobj_obj = JSON_ValueAsObject(jobj_elem->value);

            const char *const key = jobj_elem->name->string;
            const char *const name =
                JSON_ObjectGetString(jobj_obj, "name", JSON_INVALID_STRING);
            const char *const description = JSON_ObjectGetString(
                jobj_obj, "description", JSON_INVALID_STRING);

            if (key == JSON_INVALID_STRING) {
                LOG_WARNING(
                    "Invalid game string object entry %d: missing key.", i);
            } else if (name == JSON_INVALID_STRING) {
                LOG_WARNING(
                    "Invalid game string object entry %s: missing value.", key);
            } else {
                GS_OBJECT_ENTRY *const object_entry = &out_table->objects[i];
                object_entry->key = Memory_DupStr(key);
                object_entry->name = Memory_DupStr(name);
                object_entry->description = description != JSON_INVALID_STRING
                    ? Memory_DupStr(description)
                    : NULL;
            }
        }
    }

    // Load game_strings
    JSON_OBJECT *const jgs_obj = JSON_ObjectGetObject(root_obj, "game_strings");
    if (jgs_obj != NULL) {
        const size_t gs_count = jgs_obj->length;
        out_table->game_strings =
            Memory_Alloc(sizeof(GS_GAME_STRING_ENTRY) * (gs_count + 1));

        JSON_OBJECT_ELEMENT *jgs_elem = jgs_obj->start;
        for (size_t i = 0; i < gs_count; i++, jgs_elem = jgs_elem->next) {
            JSON_OBJECT *const jgs_obj = JSON_ValueAsObject(jgs_elem->value);

            const char *const key = jgs_elem->name->string;
            const char *const value =
                JSON_ValueGetString(jgs_elem->value, JSON_INVALID_STRING);

            if (key == JSON_INVALID_STRING) {
                LOG_WARNING("Invalid game string entry %d: missing key.", i);
            } else if (value == JSON_INVALID_STRING) {
                LOG_WARNING("Invalid game string entry %d: missing value.", i);
            } else {
                GS_GAME_STRING_ENTRY *const gs_entry =
                    &out_table->game_strings[i];
                gs_entry->key = Memory_DupStr(key);
                gs_entry->value = Memory_DupStr(value);
            }
        }
    }

end:
    if (!result) {
        if (out_table != NULL) {
            GS_Table_Free(out_table);
        }
    }
    return result;
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

    gs_file->level_count = jlvl_arr->length;
    gs_file->levels = Memory_Alloc(sizeof(GS_TABLE) * jlvl_arr->length);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    for (size_t i = 0; i < jlvl_arr->length; i++, jlvl_elem = jlvl_elem->next) {
        GS_TABLE *const level_table = &gs_file->levels[i];

        JSON_OBJECT *const jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (jlvl_obj == NULL) {
            LOG_ERROR("'levels' elements must be dictionaries");
            result = false;
            goto end;
        }

        result &= M_LoadTableFromJSON(jlvl_obj, level_table);
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
    result &= M_LoadTableFromJSON(root_obj, &gs_file->global);
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
