#include "game/game_flow/reader.h"

#include "game/game_flow.h"
#include "game/game_flow/common.h"
#include "game/game_flow/vars.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>
#include <libtrx/json.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

typedef void (*M_LOAD_ARRAY_FUNC)(JSON_OBJECT *, GAME_FLOW *, void *);

static void M_LoadGlobalInjections(JSON_OBJECT *obj, GAME_FLOW *gf);
static bool M_LoadGlobal(JSON_OBJECT *obj, GAME_FLOW *gf);

static bool M_LoadArray(
    JSON_OBJECT *obj, const char *key, size_t element_size,
    void (*load_func)(JSON_OBJECT *, GAME_FLOW *, void *), GAME_FLOW *const gf,
    int32_t *const count, void **const elements);

static void M_LoadLevel(
    JSON_OBJECT *obj, const GAME_FLOW *gf, GAME_FLOW_LEVEL *level);
static void M_LoadLevelInjections(
    JSON_OBJECT *obj, const GAME_FLOW *gf, GAME_FLOW_LEVEL *level);
static bool M_LoadLevels(JSON_OBJECT *obj, GAME_FLOW *gf);

static void M_LoadFMV(
    JSON_OBJECT *obj, const GAME_FLOW *gf, GAME_FLOW_FMV *level);
static bool M_LoadFMVs(JSON_OBJECT *obj, GAME_FLOW *gf);

static void M_LoadCutscene(
    JSON_OBJECT *obj, const GAME_FLOW *gf, GAME_FLOW_CUTSCENE *level);
static bool M_LoadCutscenes(JSON_OBJECT *obj, GAME_FLOW *gf);

static bool M_LoadGlobal(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    gf->title_replace =
        GF_TranslateScriptCommand(JSON_ObjectGetInt(obj, "title_replace", -1));

    gf->first_option = GF_TranslateScriptCommand(
        JSON_ObjectGetInt(obj, "first_option", 0x500));
    gf->on_death_demo_mode = GF_TranslateScriptCommand(
        JSON_ObjectGetInt(obj, "on_death_demo_mode", 0x500));
    gf->on_death_in_game = GF_TranslateScriptCommand(
        JSON_ObjectGetInt(obj, "on_death_in_game", -1));
    gf->demo_delay = JSON_ObjectGetInt(obj, "demo_delay", 30);
    gf->on_demo_interrupt = GF_TranslateScriptCommand(
        JSON_ObjectGetInt(obj, "on_demo_interrupt", 0x500));
    gf->on_demo_end =
        GF_TranslateScriptCommand(JSON_ObjectGetInt(obj, "on_demo_end", 0x500));

    gf->is_demo_version = JSON_ObjectGetBool(obj, "demo_version", false);
    gf->title_disabled = JSON_ObjectGetBool(obj, "title_disabled", false);

    // clang-format off
    gf->load_save_disabled = JSON_ObjectGetBool(obj, "load_save_disabled", false);
    gf->cheat_keys = JSON_ObjectGetBool(obj, "cheat_keys", true);
    gf->lockout_option_ring = JSON_ObjectGetBool(obj, "lockout_option_ring", true);
    gf->play_any_level = JSON_ObjectGetBool(obj, "play_any_level", false);
    gf->gym_enabled = JSON_ObjectGetBool(obj, "gym_enabled", true);
    gf->single_level = JSON_ObjectGetInt(obj, "single_level", -1);
    // clang-format on

    gf->title_track = JSON_ObjectGetInt(obj, "title_track", MX_INACTIVE);
    gf->secret_track = JSON_ObjectGetInt(obj, "secret_track", MX_INACTIVE);
    gf->level_complete_track =
        JSON_ObjectGetInt(obj, "level_complete_track", MX_INACTIVE);

    M_LoadGlobalInjections(obj, gf);
    return true;
}

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

static bool M_LoadArray(
    JSON_OBJECT *const obj, const char *const key, const size_t element_size,
    M_LOAD_ARRAY_FUNC load_func, GAME_FLOW *const gf, int32_t *const count,
    void **const elements)
{
    bool result = true;

    JSON_ARRAY *const elem_arr = JSON_ObjectGetArray(obj, key);
    if (elem_arr == NULL) {
        LOG_ERROR("'%s' must be a list", key);
        result = false;
        goto end;
    }

    *count = elem_arr->length;
    *elements = Memory_Alloc(element_size * (*count));

    JSON_ARRAY_ELEMENT *elem = elem_arr->start;
    for (size_t i = 0; i < elem_arr->length; i++, elem = elem->next) {
        void *const element = (char *)*elements + i * element_size;

        JSON_OBJECT *const elem_obj = JSON_ValueAsObject(elem->value);
        if (elem_obj == NULL) {
            LOG_ERROR("'%s' elements must be dictionaries", key);
            result = false;
            goto end;
        }

        load_func(elem_obj, gf, element);
    }

end:
    return result;
}

static void M_LoadLevel(
    JSON_OBJECT *const obj, const GAME_FLOW *const gf,
    GAME_FLOW_LEVEL *const level)
{
    const char *const level_type = JSON_ObjectGetString(obj, "type", NULL);
    if (level_type == NULL) {
        level->demo = false;
    } else if (strcmp(level_type, "demo") == 0) {
        level->demo = true;
    } else {
        Shell_ExitSystemFmt("Invalid level type: '%s'", level_type);
    }

    const char *const path = JSON_ObjectGetString(obj, "path", NULL);
    if (path == NULL) {
        Shell_ExitSystemFmt("Missing level path");
    }
    level->path = Memory_DupStr(path);

    M_LoadLevelInjections(obj, gf, level);
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

static bool M_LoadLevels(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    return M_LoadArray(
        obj, "levels", sizeof(GAME_FLOW_LEVEL), (M_LOAD_ARRAY_FUNC)M_LoadLevel,
        gf, &gf->level_count, (void **)&gf->levels);
}

static void M_LoadFMV(
    JSON_OBJECT *const obj, const GAME_FLOW *const gf, GAME_FLOW_FMV *const fmv)
{
    const char *const path = JSON_ObjectGetString(obj, "path", NULL);
    if (path == NULL) {
        Shell_ExitSystemFmt("Missing FMV path");
    }
    fmv->path = Memory_DupStr(path);
}

static bool M_LoadFMVs(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    return M_LoadArray(
        obj, "fmvs", sizeof(GAME_FLOW_FMV), (M_LOAD_ARRAY_FUNC)M_LoadFMV, gf,
        &gf->fmv_count, (void **)&gf->fmvs);
}

static void M_LoadCutscene(
    JSON_OBJECT *const obj, const GAME_FLOW *const gf,
    GAME_FLOW_CUTSCENE *const cutscene)
{
    const char *const path = JSON_ObjectGetString(obj, "path", NULL);
    if (path == NULL) {
        Shell_ExitSystemFmt("Missing cutscene path");
    }
    cutscene->path = Memory_DupStr(path);
}

static bool M_LoadCutscenes(JSON_OBJECT *obj, GAME_FLOW *const gf)
{
    return M_LoadArray(
        obj, "cutscenes", sizeof(GAME_FLOW_CUTSCENE),
        (M_LOAD_ARRAY_FUNC)M_LoadCutscene, gf, &gf->cutscene_count,
        (void **)&gf->cutscenes);
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
    result &= M_LoadGlobal(root_obj, gf);
    result &= M_LoadLevels(root_obj, gf);
    result &= M_LoadCutscenes(root_obj, gf);
    result &= M_LoadFMVs(root_obj, gf);

    if (gf->level_count != g_LegacyLevelCount) {
        LOG_ERROR(
            "'levels' must have exactly %d levels, as we still rely on legacy "
            "tombpc.dat",
            g_LegacyLevelCount);
        result = false;
        goto end;
    }

end:
    if (root != NULL) {
        JSON_ValueFree(root);
        root = NULL;
    }

    if (!result) {
        GF_N_Shutdown();
    }

    gf->demo_level_count = 0;
    for (int32_t i = 0; i < gf->level_count; i++) {
        if (gf->levels[i].demo) {
            gf->demo_level_count++;
        }
    }
    gf->demo_levels = Memory_Alloc(sizeof(int32_t) * gf->demo_level_count);
    int32_t count = 0;
    for (int32_t i = 0; i < gf->level_count; i++) {
        if (gf->levels[i].demo) {
            gf->demo_levels[count++] = i;
        }
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
        Memory_FreePointer(&gf->levels[i].path);
        Memory_FreePointer(&gf->levels[i].title);
    }
    Memory_FreePointer(&gf->levels);
    gf->level_count = 0;

    Memory_FreePointer(&gf->demo_levels);
    gf->demo_level_count = 0;

    for (int32_t i = 0; i < gf->cutscene_count; i++) {
        Memory_FreePointer(&gf->cutscenes[i].path);
    }
    Memory_FreePointer(&gf->cutscenes);
    gf->cutscene_count = 0;

    for (int32_t i = 0; i < gf->fmv_count; i++) {
        Memory_FreePointer(&gf->fmvs[i].path);
    }
    Memory_FreePointer(&gf->fmvs);
    gf->fmv_count = 0;
}
