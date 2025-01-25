#include "filesystem.h"
#include "game/game_flow.h"
#include "game/game_string_table.h"
#include "game/game_string_table/priv.h"
#include "game/shell.h"
#include "json.h"
#include "log.h"
#include "memory.h"

static void M_LoadTableFromJSON(JSON_OBJECT *root_obj, GS_TABLE *out_table);
static void M_LoadLevelsFromJSON(
    JSON_OBJECT *obj, const char *key, GAME_FLOW_LEVEL_TYPE level_type,
    GS_LEVEL_TABLE *gs_level_table);

static void M_LoadTableFromJSON(
    JSON_OBJECT *const root_obj, GS_TABLE *const out_table)
{
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
}

static void M_LoadLevelsFromJSON(
    JSON_OBJECT *const obj, const char *const key,
    const GAME_FLOW_LEVEL_TYPE level_type, GS_LEVEL_TABLE *const gs_level_table)
{
    JSON_ARRAY *const jlvl_arr = JSON_ObjectGetArray(obj, key);
    if (jlvl_arr == NULL) {
        Shell_ExitSystemFmt("'%s' must be a list", key);
        return;
    }

    if (jlvl_arr->length != (size_t)GF_GetLevelCount(level_type)) {
        Shell_ExitSystemFmt(
            "'%s' length must match with the game flow level count (got: "
            "%d, expected: %d)",
            key, jlvl_arr->length, GF_GetLevelCount(level_type));
    }

    gs_level_table->count = jlvl_arr->length;
    gs_level_table->entries = Memory_Alloc(sizeof(GS_LEVEL) * jlvl_arr->length);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    for (size_t i = 0; i < jlvl_arr->length; i++, jlvl_elem = jlvl_elem->next) {
        GS_LEVEL *const level = &gs_level_table->entries[i];

        JSON_OBJECT *const jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (jlvl_obj == NULL) {
            Shell_ExitSystem("'levels' elements must be dictionaries");
            return;
        }

        const char *const title =
            JSON_ObjectGetString(jlvl_obj, "title", JSON_INVALID_STRING);
        if (title == JSON_INVALID_STRING) {
            Shell_ExitSystemFmt("Level %d is missing title.", i);
            return;
        }
        level->title = Memory_DupStr(title);

        M_LoadTableFromJSON(jlvl_obj, &level->table);
    }
}

void GameStringTable_LoadFromFile(const char *const path)
{
    GameStringTable_Shutdown();

    JSON_VALUE *root = NULL;

    char *script_data = NULL;
    if (!File_Load(path, &script_data, NULL)) {
        Shell_ExitSystemFmt("failed to open strings file (path: %d)", path);
    }

    JSON_PARSE_RESULT parse_result;
    root = JSON_ParseEx(
        script_data, strlen(script_data), JSON_PARSE_FLAGS_ALLOW_JSON5, NULL,
        NULL, &parse_result);
    if (root == NULL) {
        Shell_ExitSystemFmt(
            "Failed to parse script file: %s in line %d, char %d",
            JSON_GetErrorDescription(parse_result.error),
            parse_result.error_line_no, parse_result.error_row_no, script_data);
    }

    GS_FILE *const gs_file = &g_GST_File;
    JSON_OBJECT *root_obj = JSON_ValueAsObject(root);
    M_LoadTableFromJSON(root_obj, &gs_file->global);
    M_LoadLevelsFromJSON(root_obj, "levels", GFL_NORMAL, &gs_file->levels);
    M_LoadLevelsFromJSON(root_obj, "demos", GFL_DEMO, &gs_file->demos);
#if TR_VERSION == 2
    // TODO: TR1 still has everything in a single linear sequence
    M_LoadLevelsFromJSON(
        root_obj, "cutscenes", GFL_CUTSCENE, &gs_file->cutscenes);
#endif

    if (root != NULL) {
        JSON_ValueFree(root);
        root = NULL;
    }
    Memory_FreePointer(&script_data);
}
