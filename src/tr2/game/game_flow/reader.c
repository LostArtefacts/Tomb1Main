#include "game/game_flow/reader.h"

#include "game/game_flow/common.h"
#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>
#include <libtrx/json.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

static void M_LoadGlobalInjections(JSON_OBJECT *obj, GAME_FLOW *gf);
static void M_LoadLevelInjections(
    JSON_OBJECT *obj, const GAME_FLOW *gf, GAME_FLOW_LEVEL *level);
static bool M_LoadScriptLevels(JSON_OBJECT *obj, GAME_FLOW *gf);

static void M_LoadGlobalInjections(JSON_OBJECT *const obj, GAME_FLOW *const gf)
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
    JSON_OBJECT *const obj, const GAME_FLOW *const gf,
    GAME_FLOW_LEVEL *const level)
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

static bool M_LoadScriptLevels(JSON_OBJECT *obj, GAME_FLOW *const gf)
{
    bool result = true;

    JSON_ARRAY *const jlvl_arr = JSON_ObjectGetArray(obj, "levels");
    if (jlvl_arr == NULL) {
        LOG_ERROR("'levels' must be a list");
        result = false;
        goto end;
    }

    int32_t level_count = jlvl_arr->length;
    if (level_count != g_GameFlowLegacy.num_levels) {
        LOG_ERROR(
            "'levels' must have exactly %d levels, as we still rely on legacy "
            "tombpc.dat",
            g_GameFlowLegacy.num_levels);
        result = false;
        goto end;
    }

    gf->level_count = level_count;
    gf->levels = Memory_Alloc(sizeof(GAME_FLOW_LEVEL) * level_count);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    for (size_t i = 0; i < jlvl_arr->length; i++, jlvl_elem = jlvl_elem->next) {
        GAME_FLOW_LEVEL *const level = &gf->levels[i];

        JSON_OBJECT *const jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (jlvl_obj == NULL) {
            LOG_ERROR("'levels' elements must be dictionaries");
            result = false;
            goto end;
        }

        M_LoadLevelInjections(jlvl_obj, gf, level);
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

    GAME_FLOW *const gf = &g_GameFlow;
    JSON_OBJECT *root_obj = JSON_ValueAsObject(root);
    M_LoadGlobalInjections(root_obj, gf);
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
    GAME_FLOW *const gf = &g_GameFlow;

    for (int32_t i = 0; i < gf->injections.count; i++) {
        Memory_FreePointer(&gf->injections.data_paths[i]);
    }
    Memory_FreePointer(&gf->injections.data_paths);

    for (int32_t i = 0; i < gf->level_count; i++) {
        for (int32_t j = 0; j < gf->levels[i].injections.count; j++) {
            Memory_FreePointer(&gf->levels[i].injections.data_paths[j]);
        }
        Memory_FreePointer(&gf->levels[i].injections.data_paths);
    }
    Memory_FreePointer(&gf->levels);
    gf->level_count = 0;
}
