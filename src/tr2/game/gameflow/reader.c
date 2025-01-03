#include "game/gameflow/reader.h"

#include "game/gameflow/gameflow_new.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>
#include <libtrx/json.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

static void M_LoadGlobalInjections(JSON_OBJECT *obj, GAME_FLOW_NEW *gf);
static void M_LoadLevelInjections(
    JSON_OBJECT *obj, const GAME_FLOW_NEW *gf, GAME_FLOW_NEW_LEVEL *level);
static void M_StringTableShutdown(GAME_FLOW_NEW_STRING_ENTRY *dest);
static bool M_LoadStringTable(
    JSON_OBJECT *root_obj, const char *key, GAME_FLOW_NEW_STRING_ENTRY **dest);
static bool M_LoadScriptLevels(JSON_OBJECT *obj, GAME_FLOW_NEW *gf);

static void M_LoadGlobalInjections(
    JSON_OBJECT *const obj, GAME_FLOW_NEW *const gf)
{
    gf->injections.count = 0;
    JSON_ARRAY *const injections = JSON_ObjectGetArray(obj, "injections");
    if (injections == NULL) {
        return;
    }

    gf->injections.count = injections->length;
    gf->injections.data_paths =
        Memory_Alloc(sizeof(char *) * injections->length);
    for (size_t i = 0; i < injections->length; i++) {
        const char *const str = JSON_ArrayGetString(injections, i, NULL);
        gf->injections.data_paths[i] = Memory_DupStr(str);
    }
}

static void M_LoadLevelInjections(
    JSON_OBJECT *const obj, const GAME_FLOW_NEW *const gf,
    GAME_FLOW_NEW_LEVEL *const level)
{
    const bool inherit = JSON_ObjectGetBool(obj, "inherit_injections", true);
    JSON_ARRAY *const injections = JSON_ObjectGetArray(obj, "injections");

    level->injections.count = 0;
    if (injections == NULL && !inherit) {
        return;
    }

    if (inherit) {
        level->injections.count += gf->injections.count;
    }
    if (injections != NULL) {
        level->injections.count += injections->length;
    }

    level->injections.data_paths =
        Memory_Alloc(sizeof(char *) * level->injections.count);
    int32_t base_index = 0;

    if (inherit) {
        for (int32_t i = 0; i < gf->injections.count; i++) {
            level->injections.data_paths[i] =
                Memory_DupStr(gf->injections.data_paths[i]);
        }
        base_index = gf->injections.count;
    }

    if (injections == NULL) {
        return;
    }

    for (size_t i = 0; i < injections->length; i++) {
        const char *const str = JSON_ArrayGetString(injections, i, NULL);
        level->injections.data_paths[base_index + i] = Memory_DupStr(str);
    }
}

static void M_StringTableShutdown(GAME_FLOW_NEW_STRING_ENTRY *const dest)
{
    if (dest == NULL) {
        return;
    }
    GAME_FLOW_NEW_STRING_ENTRY *cur = dest;
    while (cur->key != NULL) {
        Memory_FreePointer(&cur->key);
        Memory_FreePointer(&cur->value);
        cur++;
    }
    Memory_Free(dest);
}

static bool M_LoadStringTable(
    JSON_OBJECT *const root_obj, const char *const key,
    GAME_FLOW_NEW_STRING_ENTRY **dest)
{
    const JSON_OBJECT *const strings_obj = JSON_ObjectGetObject(root_obj, key);
    if (strings_obj == NULL) {
        // key is missing - rely on default strings
        return true;
    }

    *dest = Memory_Alloc(
        sizeof(GAME_FLOW_NEW_STRING_ENTRY) * (strings_obj->length + 1));

    GAME_FLOW_NEW_STRING_ENTRY *cur = *dest;
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

static bool M_LoadScriptLevels(JSON_OBJECT *obj, GAME_FLOW_NEW *const gf)
{
    bool result = true;

    JSON_ARRAY *const jlvl_arr = JSON_ObjectGetArray(obj, "levels");
    if (jlvl_arr == NULL) {
        LOG_ERROR("'levels' must be a list");
        result = false;
        goto end;
    }

    int32_t level_count = jlvl_arr->length;
    if (level_count != g_GameFlow.num_levels) {
        LOG_ERROR(
            "'levels' must have exactly %d levels, as we still rely on legacy "
            "tombpc.dat",
            g_GameFlow.num_levels);
        result = false;
        goto end;
    }

    gf->level_count = level_count;
    gf->levels = Memory_Alloc(sizeof(GAME_FLOW_NEW_LEVEL) * level_count);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    for (size_t i = 0; i < jlvl_arr->length; i++, jlvl_elem = jlvl_elem->next) {
        GAME_FLOW_NEW_LEVEL *const level = &gf->levels[i];

        JSON_OBJECT *const jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (jlvl_obj == NULL) {
            LOG_ERROR("'levels' elements must be dictionaries");
            result = false;
            goto end;
        }

        M_LoadLevelInjections(jlvl_obj, gf, level);

        result &= M_LoadStringTable(
            jlvl_obj, "object_strings", &level->object_strings);
        result &=
            M_LoadStringTable(jlvl_obj, "game_strings", &level->game_strings);
    }

end:
    return result;
}

bool GF_N_Load(const char *const path)
{
    GF_N_Shutdown();

    bool result = true;
    JSON_VALUE *root = NULL;

    char *script_data = NULL;
    if (!File_Load(path, &script_data, NULL)) {
        LOG_ERROR("failed to open script file");
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

    GAME_FLOW_NEW *const gf = &g_GameFlowNew;
    JSON_OBJECT *root_obj = JSON_ValueAsObject(root);
    M_LoadGlobalInjections(root_obj, gf);
    result &=
        M_LoadStringTable(root_obj, "object_strings", &gf->object_strings);
    result &= M_LoadStringTable(root_obj, "game_strings", &gf->game_strings);
    result &= M_LoadScriptLevels(root_obj, gf);

end:
    if (root != NULL) {
        JSON_ValueFree(root);
        root = NULL;
    }

    if (!result) {
        GF_N_Shutdown();
    }

    Memory_FreePointer(&script_data);
    return result;
}

void GF_N_Shutdown(void)
{
    GAME_FLOW_NEW *const gf = &g_GameFlowNew;

    for (int32_t i = 0; i < gf->injections.count; i++) {
        Memory_FreePointer(&gf->injections.data_paths[i]);
    }
    Memory_FreePointer(&gf->injections.data_paths);

    for (int32_t i = 0; i < gf->level_count; i++) {
        M_StringTableShutdown(gf->levels[i].object_strings);
        M_StringTableShutdown(gf->levels[i].game_strings);

        for (int32_t j = 0; j < gf->levels[i].injections.count; j++) {
            Memory_FreePointer(&gf->levels[i].injections.data_paths[j]);
        }
        Memory_FreePointer(&gf->levels[i].injections.data_paths);
    }

    M_StringTableShutdown(gf->object_strings);
    M_StringTableShutdown(gf->game_strings);
}
