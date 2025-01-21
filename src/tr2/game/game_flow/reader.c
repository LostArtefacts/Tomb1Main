#include "game/game_flow/reader.h"

#include "game/game_flow.h"
#include "game/game_flow/common.h"
#include "game/game_flow/vars.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/enum_map.h>
#include <libtrx/filesystem.h>
#include <libtrx/json.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

typedef int32_t (*M_SEQUENCE_EVENT_HANDLER_FUNC)(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);

typedef struct {
    GAME_FLOW_SEQUENCE_EVENT_TYPE event_type;
    M_SEQUENCE_EVENT_HANDLER_FUNC handler_func;
    void *handler_func_arg;
} M_SEQUENCE_EVENT_HANDLER;

typedef void (*M_LOAD_ARRAY_FUNC)(JSON_OBJECT *, GAME_FLOW *, void *);

static GAME_FLOW_COMMAND M_LoadCommand(
    JSON_OBJECT *jcmd, GAME_FLOW_COMMAND fallback);

static int32_t M_HandleIntEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static int32_t M_HandleAddItemEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static size_t M_LoadSequenceEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data);
static void M_LoadSequence(JSON_ARRAY *jarr, GAME_FLOW_SEQUENCE *sequence);

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

static void M_LoadTitleLevel(JSON_OBJECT *obj, GAME_FLOW *gf);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);
static void M_FreeLevel(GAME_FLOW_LEVEL *level);
static void M_FreeLevels(GAME_FLOW *gf);
static void M_FreeDemos(GAME_FLOW *gf);
static void M_FreeCutscenes(GAME_FLOW *gf);
static void M_FreeFMVs(GAME_FLOW *gf);

static M_SEQUENCE_EVENT_HANDLER m_SequenceEventHandlers[] = {
    // clang-format off
    // Events without arguments
    { GFS_ENABLE_SUNSET,       NULL, NULL },
    { GFS_ENABLE_DEADLY_WATER, NULL, NULL },
    { GFS_REMOVE_WEAPONS,      NULL, NULL },
    { GFS_REMOVE_AMMO,         NULL, NULL },
    { GFS_LEVEL_COMPLETE,      NULL, NULL },
    { GFS_GAME_COMPLETE,       NULL, NULL },

    // Events with integer arguments
    { GFS_SET_MUSIC_TRACK,     M_HandleIntEvent, "music_track" },
    { GFS_SET_NUM_SECRETS,     M_HandleIntEvent, "count" },
    { GFS_SET_CAMERA_ANGLE,    M_HandleIntEvent, "anim" },
    { GFS_SET_START_ANIM,      M_HandleIntEvent, "anim" },
    { GFS_PLAY_LEVEL,          M_HandleIntEvent, "level_num" },
    { GFS_PLAY_CUTSCENE,       M_HandleIntEvent, "cutscene_num" },
    { GFS_PLAY_FMV,            M_HandleIntEvent, "fmv_num" },
    { GFS_PLAY_DEMO,           M_HandleIntEvent, "demo_num" },
    { GFS_DISABLE_FLOOR,       M_HandleIntEvent, "height" },

    // Special cases with custom handlers
    { GFS_ADD_ITEM,            M_HandleAddItemEvent, NULL },
    { GFS_ADD_SECRET_REWARD,   M_HandleAddItemEvent, NULL },

    // Sentinel to mark the end of the table
    { (GAME_FLOW_SEQUENCE_EVENT_TYPE)-1, NULL, NULL },
    // clang-format on
};

static GAME_FLOW_COMMAND M_LoadCommand(
    JSON_OBJECT *const jcmd, const GAME_FLOW_COMMAND fallback)
{
    if (jcmd == NULL) {
        return fallback;
    }

    const char *const action_str =
        JSON_ObjectGetString(jcmd, "action", JSON_INVALID_STRING);
    const int32_t param = JSON_ObjectGetInt(jcmd, "param", -1);
    if (action_str == JSON_INVALID_STRING) {
        LOG_ERROR("Unknown game flow action: %s", action_str);
        return fallback;
    }

    const GAME_FLOW_ACTION action =
        ENUM_MAP_GET(GAME_FLOW_ACTION, action_str, (GAME_FLOW_ACTION)-1234);
    if (action == (GAME_FLOW_ACTION)-1234) {
        LOG_ERROR("Unknown game flow action: %s", action_str);
        return fallback;
    }

    return (GAME_FLOW_COMMAND) { .action = action, .param = param };
}

static int32_t M_HandleIntEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg)
{
    if (event != NULL) {
        event->data =
            (void *)(intptr_t)JSON_ObjectGetInt(event_obj, user_arg, -1);
    }
    return 0;
}

static int32_t M_HandleAddItemEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg)
{
    if (event != NULL) {
        GFS_ADD_ITEM_DATA *const event_data = extra_data;
        event_data->item = JSON_ObjectGetInt(event_obj, "item", 0);
        event_data->inv_type =
            event->type == GFS_ADD_ITEM ? GF_INV_REGULAR : GF_INV_SECRET;
        event->data = event_data;
    }
    return sizeof(GFS_ADD_ITEM_DATA);
}

static size_t M_LoadSequenceEvent(
    JSON_OBJECT *const event_obj, GAME_FLOW_SEQUENCE_EVENT *const event,
    void *const extra_data)
{
    const char *const type_str = JSON_ObjectGetString(event_obj, "type", "");
    const GAME_FLOW_SEQUENCE_EVENT_TYPE type =
        ENUM_MAP_GET(GAME_FLOW_SEQUENCE_EVENT_TYPE, type_str, -1);

    const M_SEQUENCE_EVENT_HANDLER *handler = m_SequenceEventHandlers;
    while (handler->event_type != (GAME_FLOW_SEQUENCE_EVENT_TYPE)-1
           && handler->event_type != type) {
        handler++;
    }

    if (handler->event_type != type) {
        LOG_ERROR("Unknown game flow sequence event type: '%s'", type);
        return -1;
    }

    int32_t extra_data_size = 0;
    if (handler->handler_func != NULL) {
        extra_data_size = handler->handler_func(
            event_obj, NULL, NULL, handler->handler_func_arg);
    }
    if (extra_data_size >= 0 && event != NULL) {
        event->type = handler->event_type;
        LOG_DEBUG("wtf %s %p", type_str, handler->handler_func);
        if (handler->handler_func != NULL) {
            handler->handler_func(
                event_obj, event, extra_data, handler->handler_func_arg);
        } else {
            event->data = NULL;
        }
    }
    return extra_data_size;
}

static void M_LoadSequence(
    JSON_ARRAY *const jarr, GAME_FLOW_SEQUENCE *const sequence)
{
    if (jarr == NULL) {
        Shell_ExitSystem("Level has no sequence");
    }

    sequence->length = 0;
    size_t event_base_size = sizeof(GAME_FLOW_SEQUENCE_EVENT);
    size_t total_data_size = 0;
    for (size_t i = 0; i < jarr->length; i++) {
        JSON_OBJECT *jevent = JSON_ArrayGetObject(jarr, i);
        const int32_t event_extra_size =
            M_LoadSequenceEvent(jevent, NULL, NULL);
        if (event_extra_size < 0) {
            // Parsing this event failed - discard it
            continue;
        }
        total_data_size += event_base_size;
        total_data_size += event_extra_size;
        sequence->length++;
    }

    char *const data = Memory_Alloc(total_data_size);
    char *extra_data_ptr = data + event_base_size * sequence->length;
    sequence->events = (GAME_FLOW_SEQUENCE_EVENT *)data;

    int32_t j = 0;
    for (int32_t i = 0; i < sequence->length; i++) {
        JSON_OBJECT *const jevent = JSON_ArrayGetObject(jarr, i);
        const int32_t event_extra_size =
            M_LoadSequenceEvent(jevent, &sequence->events[j++], extra_data_ptr);
        if (event_extra_size < 0) {
            // Parsing this event failed - discard it
            continue;
        }
        extra_data_ptr += event_extra_size;
    }
}

static bool M_LoadGlobal(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    gf->cmd_init = M_LoadCommand(
        JSON_ObjectGetObject(obj, "cmd_init"),
        (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE });
    gf->cmd_title = M_LoadCommand(
        JSON_ObjectGetObject(obj, "cmd_title"),
        (GAME_FLOW_COMMAND) { .action = GF_NOOP });
    gf->cmd_death_demo_mode = M_LoadCommand(
        JSON_ObjectGetObject(obj, "cmd_death_demo_mode"),
        (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE });
    gf->cmd_death_in_game = M_LoadCommand(
        JSON_ObjectGetObject(obj, "cmd_death_in_game"),
        (GAME_FLOW_COMMAND) { .action = GF_NOOP });
    gf->cmd_demo_interrupt = M_LoadCommand(
        JSON_ObjectGetObject(obj, "cmd_demo_interrupt"),
        (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE });
    gf->cmd_demo_end = M_LoadCommand(
        JSON_ObjectGetObject(obj, "cmd_demo_end"),
        (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE });

    gf->is_demo_version = JSON_ObjectGetBool(obj, "demo_version", false);

    // clang-format off
    gf->demo_delay = JSON_ObjectGetInt(obj, "demo_delay", 30);
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

static void M_LoadLevelSequence(
    JSON_OBJECT *const obj, GAME_FLOW_LEVEL *const level)
{
    JSON_ARRAY *const jarr = JSON_ObjectGetArray(obj, "sequence");
    if (jarr == NULL) {
        Shell_ExitSystem("Level has no sequence");
    }

    M_LoadSequence(jarr, &level->sequence);
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

    M_LoadLevelSequence(obj, level);

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

static void M_LoadTitleLevel(JSON_OBJECT *obj, GAME_FLOW *const gf)
{
    JSON_OBJECT *title_obj = JSON_ObjectGetObject(obj, "title");
    if (title_obj != NULL) {
        gf->title_level = Memory_Alloc(sizeof(GAME_FLOW_LEVEL));
        M_LoadLevel(title_obj, gf, gf->title_level);
    }
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
    M_LoadTitleLevel(root_obj, gf);

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

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    Memory_Free(sequence);
}

static void M_FreeLevel(GAME_FLOW_LEVEL *const level)
{
    for (int32_t j = 0; j < level->injections.count; j++) {
        Memory_FreePointer(&level->injections.data_paths[j]);
    }
    M_FreeSequence(&level->sequence);
    Memory_FreePointer(&level->injections.data_paths);
    Memory_FreePointer(&level->path);
    Memory_FreePointer(&level->title);
}

static void M_FreeLevels(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->level_count; i++) {
        M_FreeLevel(&gf->levels[i]);
    }
    Memory_FreePointer(&gf->levels);
    gf->level_count = 0;
}

static void M_FreeDemos(GAME_FLOW *const gf)
{
    Memory_FreePointer(&gf->demo_levels);
    gf->demo_level_count = 0;
}

static void M_FreeCutscenes(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->cutscene_count; i++) {
        Memory_FreePointer(&gf->cutscenes[i].path);
    }
    Memory_FreePointer(&gf->cutscenes);
    gf->cutscene_count = 0;
}

static void M_FreeFMVs(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->fmv_count; i++) {
        Memory_FreePointer(&gf->fmvs[i].path);
    }
    Memory_FreePointer(&gf->fmvs);
    gf->fmv_count = 0;
}

void GF_N_Shutdown(void)
{
    GAME_FLOW *const gf = &g_GameFlow;

    for (int32_t i = 0; i < gf->injections.count; i++) {
        Memory_FreePointer(&gf->injections.data_paths[i]);
    }
    Memory_FreePointer(&gf->injections.data_paths);

    M_FreeLevels(gf);
    M_FreeDemos(gf);
    M_FreeCutscenes(gf);
    M_FreeFMVs(gf);

    if (gf->title_level != NULL) {
        M_FreeLevel(gf->title_level);
        Memory_FreePointer(&gf->title_level);
    }
}
