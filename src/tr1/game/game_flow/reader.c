#include "game/game_flow/reader.h"

#include "game/game_flow/common.h"
#include "game/game_flow/types.h"
#include "game/game_flow/vars.h"
#include "game/shell.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/enum_map.h>
#include <libtrx/filesystem.h>
#include <libtrx/json.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

#include <string.h>

typedef int32_t (*M_SEQUENCE_EVENT_HANDLER_FUNC)(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);

typedef struct {
    GAME_FLOW_SEQUENCE_EVENT_TYPE event_type;
    M_SEQUENCE_EVENT_HANDLER_FUNC handler_func;
    void *handler_func_arg;
} M_SEQUENCE_EVENT_HANDLER;

typedef void (*M_LOAD_ARRAY_FUNC)(
    JSON_OBJECT *source_elem, GAME_FLOW *gf, void *target_elem,
    size_t target_elem_idx, void *user_arg);

static void M_LoadArray(
    JSON_OBJECT *obj, const char *key, size_t element_size,
    M_LOAD_ARRAY_FUNC load_func, GAME_FLOW *const gf, int32_t *const count,
    void **const elements, void *user_arg);

static bool M_IsLegacySequence(const char *type_str);
static int32_t M_HandleIntEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static int32_t M_HandlePictureEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static int32_t M_HandleTotalStatsEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static int32_t M_HandleAddItemEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static int32_t M_HandleMeshSwapEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg);
static void M_LoadSettings(
    JSON_OBJECT *obj, GAME_FLOW_LEVEL_SETTINGS *settings);
static void M_LoadLevelSequence(
    JSON_OBJECT *obj, GAME_FLOW *gf, int32_t level_num);
static void M_LoadLevels(JSON_OBJECT *obj, GAME_FLOW *gf);

static void M_LoadFMV(
    JSON_OBJECT *obj, const GAME_FLOW *gf, GAME_FLOW_FMV *level, size_t idx,
    void *user_arg);
static void M_LoadFMVs(JSON_OBJECT *obj, GAME_FLOW *gf);

static void M_LoadRoot(JSON_OBJECT *obj, GAME_FLOW *gf);

static GAME_FLOW_LEVEL_SETTINGS m_DefaultSettings = {
    .water_color = { .r = 0.6, .g = 0.7, .b = 1.0 },
    .draw_distance_fade = 12.0f,
    .draw_distance_max = 20.0f,
};

static M_SEQUENCE_EVENT_HANDLER m_SequenceEventHandlers[] = {
    // clang-format off
    // Events without arguments
    { GFS_FLIP_MAP,         NULL, NULL },
    { GFS_REMOVE_WEAPONS,   NULL, NULL },
    { GFS_REMOVE_SCIONS,    NULL, NULL },
    { GFS_REMOVE_AMMO,      NULL, NULL },
    { GFS_REMOVE_MEDIPACKS, NULL, NULL },
    { GFS_EXIT_TO_TITLE,    NULL, NULL },

    // Events with integer arguments
    { GFS_LOAD_LEVEL,       M_HandleIntEvent, "level_id" },
    { GFS_PLAY_LEVEL,       M_HandleIntEvent, "level_id" },
    { GFS_PLAY_FMV,         M_HandleIntEvent, "fmv_num" },
    { GFS_LEVEL_STATS,         M_HandleIntEvent, "level_id" },
    { GFS_EXIT_TO_LEVEL,       M_HandleIntEvent, "level_id" },
    { GFS_EXIT_TO_CINE,        M_HandleIntEvent, "level_id" },
    { GFS_SET_CAMERA_ANGLE,    M_HandleIntEvent, "value" },
    { GFS_PLAY_SYNCED_AUDIO,   M_HandleIntEvent, "audio_id" },
    { GFS_SETUP_BACON_LARA,    M_HandleIntEvent, "anchor_room" },

    // Special cases with custom handlers
    { GFS_LOADING_SCREEN,   M_HandlePictureEvent, NULL },
    { GFS_DISPLAY_PICTURE,  M_HandlePictureEvent, NULL },
    { GFS_TOTAL_STATS,      M_HandleTotalStatsEvent, NULL },
    { GFS_ADD_ITEM,         M_HandleAddItemEvent, NULL },
    { GFS_MESH_SWAP,        M_HandleMeshSwapEvent, NULL },

    // Sentinel to mark the end of the table
    { (GAME_FLOW_SEQUENCE_EVENT_TYPE)-1, NULL, NULL },
    // clang-format on
};

static void M_LoadArray(
    JSON_OBJECT *const obj, const char *const key, const size_t element_size,
    M_LOAD_ARRAY_FUNC load_func, GAME_FLOW *const gf, int32_t *const count,
    void **const elements, void *const user_arg)
{
    JSON_ARRAY *const elem_arr = JSON_ObjectGetArray(obj, key);
    if (elem_arr == NULL) {
        Shell_ExitSystemFmt("'%s' must be a list", key);
    }

    *count = elem_arr->length;
    *elements = Memory_Alloc(element_size * (*count));

    JSON_ARRAY_ELEMENT *elem = elem_arr->start;
    for (size_t i = 0; i < elem_arr->length; i++, elem = elem->next) {
        void *const element = (char *)*elements + i * element_size;

        JSON_OBJECT *const elem_obj = JSON_ValueAsObject(elem->value);
        if (elem_obj == NULL) {
            Shell_ExitSystemFmt("'%s' elements must be dictionaries", key);
        }

        load_func(elem_obj, gf, element, i, user_arg);
    }
}

static bool M_IsLegacySequence(const char *const type_str)
{
    return strcmp(type_str, "fix_pyramid_secret") == 0
        || strcmp(type_str, "stop_cine") == 0;
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

static int32_t M_HandlePictureEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg)
{
    const char *const path =
        JSON_ObjectGetString(event_obj, "picture_path", NULL);
    if (path == NULL) {
        LOG_ERROR("Missing picture path");
        return -1;
    }
    const float display_time =
        JSON_ObjectGetDouble(event_obj, "display_time", -1.0);
    if (display_time < 0.0) {
        Shell_ExitSystemFmt("'display_time' must be a positive number");
    }
    if (event != NULL) {
        GAME_FLOW_DISPLAY_PICTURE_DATA *const event_data = extra_data;
        event_data->path =
            (char *)extra_data + sizeof(GAME_FLOW_DISPLAY_PICTURE_DATA);
        event_data->display_time = display_time;
        strcpy(event_data->path, path);
        event->data = event_data;
    }
    return sizeof(GAME_FLOW_DISPLAY_PICTURE_DATA) + strlen(path) + 1;
}

static int32_t M_HandleTotalStatsEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg)
{
    const char *const path =
        JSON_ObjectGetString(event_obj, "picture_path", NULL);
    if (path == NULL) {
        LOG_ERROR("Missing picture path");
        return -1;
    }
    if (event != NULL) {
        char *const event_data = extra_data;
        strcpy(event_data, path);
        event->data = event_data;
    }
    return strlen(path) + 1;
}

static int32_t M_HandleAddItemEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg)
{
    const GAME_OBJECT_ID object_id =
        JSON_ObjectGetInt(event_obj, "object_id", JSON_INVALID_NUMBER);
    if (object_id == JSON_INVALID_NUMBER) {
        LOG_ERROR("Invalid item: %s", object_id);
        return -1;
    }
    if (event != NULL) {
        GAME_FLOW_ADD_ITEM_DATA *const event_data = extra_data;
        event_data->object_id = object_id;
        event_data->quantity = JSON_ObjectGetInt(event_obj, "quantity", 1);
        event->data = event_data;
    }
    return sizeof(GAME_FLOW_ADD_ITEM_DATA);
}

static int32_t M_HandleMeshSwapEvent(
    JSON_OBJECT *event_obj, GAME_FLOW_SEQUENCE_EVENT *event, void *extra_data,
    void *user_arg)
{
    const GAME_OBJECT_ID object1_id =
        JSON_ObjectGetInt(event_obj, "object1_id", JSON_INVALID_NUMBER);
    if (object1_id == JSON_INVALID_NUMBER) {
        Shell_ExitSystem("'object1_id' must be a number");
    }

    const GAME_OBJECT_ID object2_id =
        JSON_ObjectGetInt(event_obj, "object2_id", JSON_INVALID_NUMBER);
    if (object2_id == JSON_INVALID_NUMBER) {
        Shell_ExitSystem("'object2_id' must be a number");
    }

    const int32_t mesh_num =
        JSON_ObjectGetInt(event_obj, "mesh_id", JSON_INVALID_NUMBER);
    if (mesh_num == JSON_INVALID_NUMBER) {
        Shell_ExitSystem("'mesh_id' must be a number");
    }

    if (event != NULL) {
        GAME_FLOW_MESH_SWAP_DATA *const swap_data = extra_data;
        swap_data->object1_id = object1_id;
        swap_data->object2_id = object2_id;
        swap_data->mesh_num = mesh_num;
        event->data = swap_data;
    }
    return sizeof(GAME_FLOW_MESH_SWAP_DATA);
}

static void M_LoadSettings(
    JSON_OBJECT *const obj, GAME_FLOW_LEVEL_SETTINGS *const settings)
{
    {
        const double value = JSON_ObjectGetDouble(
            obj, "draw_distance_fade", JSON_INVALID_NUMBER);
        if (value != JSON_INVALID_NUMBER) {
            settings->draw_distance_fade = value;
        }
    }

    {
        const double value =
            JSON_ObjectGetDouble(obj, "draw_distance_max", JSON_INVALID_NUMBER);
        if (value != JSON_INVALID_NUMBER) {
            settings->draw_distance_max = value;
        }
    }

    {
        JSON_ARRAY *const tmp_arr = JSON_ObjectGetArray(obj, "water_color");
        if (tmp_arr != NULL) {
            settings->water_color.r =
                JSON_ArrayGetDouble(tmp_arr, 0, settings->water_color.r);
            settings->water_color.g =
                JSON_ArrayGetDouble(tmp_arr, 1, settings->water_color.g);
            settings->water_color.b =
                JSON_ArrayGetDouble(tmp_arr, 2, settings->water_color.b);
        }
    }
}

static size_t M_LoadSequenceEvent(
    JSON_OBJECT *const event_obj, GAME_FLOW_SEQUENCE_EVENT *const event,
    void *const extra_data)
{
    const char *const type_str = JSON_ObjectGetString(event_obj, "type", "");
    if (M_IsLegacySequence(type_str)) {
        LOG_WARNING("legacy type '%s' ignored", type_str);
        return -1;
    }

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
        if (handler->handler_func != NULL) {
            handler->handler_func(
                event_obj, event, extra_data, handler->handler_func_arg);
        } else {
            event->data = NULL;
        }
    }
    return extra_data_size;
}

static void M_LoadLevelSequence(
    JSON_OBJECT *const obj, GAME_FLOW *const gf, const int32_t level_num)
{
    JSON_ARRAY *jseq_arr = JSON_ObjectGetArray(obj, "sequence");
    if (!jseq_arr) {
        Shell_ExitSystemFmt("level %d: 'sequence' must be a list", level_num);
    }

    GAME_FLOW_SEQUENCE *const sequence = &gf->levels[level_num].sequence;

    sequence->length = 0;
    size_t event_base_size = sizeof(GAME_FLOW_SEQUENCE_EVENT);
    size_t total_data_size = 0;
    for (size_t i = 0; i < jseq_arr->length; i++) {
        JSON_OBJECT *jevent = JSON_ArrayGetObject(jseq_arr, i);
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
        JSON_OBJECT *const jevent = JSON_ArrayGetObject(jseq_arr, i);
        const int32_t event_extra_size =
            M_LoadSequenceEvent(jevent, &sequence->events[j++], extra_data_ptr);
        if (event_extra_size < 0) {
            // Parsing this event failed - discard it
            continue;
        }
        extra_data_ptr += event_extra_size;
    }

    for (int32_t i = 0; i < sequence->length; i++) {
        if ((sequence->events[i].type == GFS_PLAY_LEVEL
             || sequence->events[i].type == GFS_LOAD_LEVEL)
            && (int32_t)(intptr_t)sequence->events[i].data == -1) {
            sequence->events[i].data = (void *)(intptr_t)level_num;
        }
    }
}

static void M_LoadLevels(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    JSON_ARRAY *jlvl_arr = JSON_ObjectGetArray(obj, "levels");
    if (!jlvl_arr) {
        Shell_ExitSystem("'levels' must be a list");
    }

    int32_t level_count = jlvl_arr->length;

    gf->levels = Memory_Alloc(sizeof(GAME_FLOW_LEVEL) * level_count);
    g_GameInfo.current = Memory_Alloc(sizeof(RESUME_INFO) * level_count);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    int level_num = 0;

    gf->has_demo = 0;
    gf->gym_level_num = -1;
    gf->first_level_num = -1;
    gf->last_level_num = -1;
    gf->title_level_num = -1;
    gf->level_count = jlvl_arr->length;

    GAME_FLOW_LEVEL *level = &gf->levels[0];
    while (jlvl_elem) {
        JSON_OBJECT *jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (!jlvl_obj) {
            Shell_ExitSystem("'levels' elements must be dictionaries");
        }

        const char *tmp_s;
        int32_t tmp_i;
        JSON_ARRAY *tmp_arr;

        tmp_i = JSON_ObjectGetInt(jlvl_obj, "music", JSON_INVALID_NUMBER);
        if (tmp_i == JSON_INVALID_NUMBER) {
            Shell_ExitSystemFmt(
                "level %d: 'music' must be a number", level_num);
        }
        level->music_track = tmp_i;

        tmp_s = JSON_ObjectGetString(jlvl_obj, "file", JSON_INVALID_STRING);
        if (tmp_s == JSON_INVALID_STRING) {
            Shell_ExitSystemFmt("level %d: 'file' must be a string", level_num);
        }
        level->path = Memory_DupStr(tmp_s);

        tmp_s = JSON_ObjectGetString(jlvl_obj, "type", JSON_INVALID_STRING);
        if (tmp_s == JSON_INVALID_STRING) {
            Shell_ExitSystemFmt("level %d: 'type' must be a string", level_num);
        }

        level->num = level - gf->levels;
        level->type = ENUM_MAP_GET(GAME_FLOW_LEVEL_TYPE, tmp_s, -1);

        switch (level->type) {
        case GFL_TITLE:
        case GFL_TITLE_DEMO_PC:
            if (gf->title_level_num != -1) {
                Shell_ExitSystemFmt(
                    "level %d: there can be only one title level", level_num);
            }
            gf->title_level_num = level_num;
            break;

        case GFL_GYM:
            if (gf->gym_level_num != -1) {
                Shell_ExitSystemFmt(
                    "level %d: there can be only one gym level", level_num);
            }
            gf->gym_level_num = level_num;
            break;

        case GFL_LEVEL_DEMO_PC:
        case GFL_NORMAL:
            if (gf->first_level_num == -1) {
                gf->first_level_num = level_num;
            }
            gf->last_level_num = level_num;
            break;

        case GFL_BONUS:
        case GFL_CUTSCENE:
        case GFL_CURRENT:
            break;

        default:
            Shell_ExitSystemFmt(
                "level %d: unknown level type %s", level_num, tmp_s);
        }

        tmp_i = JSON_ObjectGetBool(jlvl_obj, "demo", JSON_INVALID_BOOL);
        if (tmp_i != JSON_INVALID_BOOL) {
            level->demo = tmp_i;
            gf->has_demo |= tmp_i;
        } else {
            level->demo = 0;
        }

        level->settings = gf->settings;
        M_LoadSettings(jlvl_obj, &level->settings);
        level->unobtainable.pickups =
            JSON_ObjectGetInt(jlvl_obj, "unobtainable_pickups", 0);

        level->unobtainable.kills =
            JSON_ObjectGetInt(jlvl_obj, "unobtainable_kills", 0);

        level->unobtainable.secrets =
            JSON_ObjectGetInt(jlvl_obj, "unobtainable_secrets", 0);

        tmp_i = JSON_ObjectGetBool(jlvl_obj, "inherit_injections", 1);
        tmp_arr = JSON_ObjectGetArray(jlvl_obj, "injections");
        if (tmp_arr) {
            level->injections.count = tmp_arr->length;
            if (tmp_i) {
                level->injections.count += gf->injections.count;
            }
            level->injections.data_paths =
                Memory_Alloc(sizeof(char *) * level->injections.count);

            int inj_base_index = 0;
            if (tmp_i) {
                for (int i = 0; i < gf->injections.count; i++) {
                    level->injections.data_paths[i] =
                        Memory_DupStr(gf->injections.data_paths[i]);
                }
                inj_base_index = gf->injections.count;
            }

            for (size_t i = 0; i < tmp_arr->length; i++) {
                const char *const str = JSON_ArrayGetString(tmp_arr, i, NULL);
                level->injections.data_paths[inj_base_index + i] =
                    Memory_DupStr(str);
            }
        } else if (tmp_i) {
            level->injections.count = gf->injections.count;
            level->injections.data_paths =
                Memory_Alloc(sizeof(char *) * level->injections.count);
            for (int i = 0; i < gf->injections.count; i++) {
                level->injections.data_paths[i] =
                    Memory_DupStr(gf->injections.data_paths[i]);
            }
        } else {
            level->injections.count = 0;
        }

        tmp_i = JSON_ObjectGetInt(jlvl_obj, "lara_type", (int32_t)O_LARA);
        if (tmp_i < 0 || tmp_i >= O_NUMBER_OF) {
            Shell_ExitSystemFmt(
                "level %d: 'lara_type' must be a valid game object id",
                level_num);
        }
        level->lara_type = (GAME_OBJECT_ID)tmp_i;

        tmp_arr = JSON_ObjectGetArray(jlvl_obj, "item_drops");
        level->item_drops.count = 0;
        if (tmp_arr && gf->enable_tr2_item_drops) {
            LOG_WARNING(
                "TR2 item drops are enabled: gameflow-defined drops for level "
                "%d will be ignored",
                level_num);
        } else if (tmp_arr) {
            level->item_drops.count = (signed)tmp_arr->length;
            level->item_drops.data = Memory_Alloc(
                sizeof(GAME_FLOW_DROP_ITEM_DATA) * (signed)tmp_arr->length);

            for (int i = 0; i < level->item_drops.count; i++) {
                GAME_FLOW_DROP_ITEM_DATA *data = &level->item_drops.data[i];
                JSON_OBJECT *jlvl_data = JSON_ArrayGetObject(tmp_arr, i);

                data->enemy_num = JSON_ObjectGetInt(
                    jlvl_data, "enemy_num", JSON_INVALID_NUMBER);
                if (data->enemy_num == JSON_INVALID_NUMBER) {
                    Shell_ExitSystemFmt(
                        "level %d, item drop %d: 'enemy_num' must be a number",
                        level_num, i);
                }

                JSON_ARRAY *object_arr =
                    JSON_ObjectGetArray(jlvl_data, "object_ids");
                if (!object_arr) {
                    Shell_ExitSystemFmt(
                        "level %d, item drop %d: 'object_ids' must be an array",
                        level_num, i);
                }

                data->count = (signed)object_arr->length;
                data->object_ids = Memory_Alloc(sizeof(int16_t) * data->count);
                for (int j = 0; j < data->count; j++) {
                    int id = JSON_ArrayGetInt(object_arr, j, -1);
                    if (id < 0 || id >= O_NUMBER_OF) {
                        Shell_ExitSystemFmt(
                            "level %d, item drop %d, index %d: 'object_id' "
                            "must be a valid object id",
                            level_num, i, j);
                    }
                    data->object_ids[j] = (int16_t)id;
                }
            }
        }

        M_LoadLevelSequence(jlvl_obj, gf, level_num);

        jlvl_elem = jlvl_elem->next;
        level_num++;
        level++;
    }

    if (gf->title_level_num == -1) {
        Shell_ExitSystem("at least one level must be of title type");
    }
    if (gf->first_level_num == -1 || gf->last_level_num == -1) {
        Shell_ExitSystem("at least one level must be of normal type");
    }
}

static void M_LoadFMV(
    JSON_OBJECT *const obj, const GAME_FLOW *const gf, GAME_FLOW_FMV *const fmv,
    size_t idx, void *const user_arg)
{
    const char *const path = JSON_ObjectGetString(obj, "path", NULL);
    if (path == NULL) {
        Shell_ExitSystemFmt("Missing FMV path");
    }
    fmv->path = Memory_DupStr(path);
}

static void M_LoadFMVs(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    M_LoadArray(
        obj, "fmvs", sizeof(GAME_FLOW_FMV), (M_LOAD_ARRAY_FUNC)M_LoadFMV, gf,
        &gf->fmv_count, (void **)&gf->fmvs, NULL);
}

static void M_LoadRoot(JSON_OBJECT *const obj, GAME_FLOW *const gf)
{
    const char *tmp_s;
    int tmp_i;
    double tmp_d;
    JSON_ARRAY *tmp_arr;

    tmp_s = JSON_ObjectGetString(obj, "main_menu_picture", JSON_INVALID_STRING);
    if (tmp_s == JSON_INVALID_STRING) {
        Shell_ExitSystem("'main_menu_picture' must be a string");
    }
    gf->main_menu_background_path = Memory_DupStr(tmp_s);

    tmp_s =
        JSON_ObjectGetString(obj, "savegame_fmt_legacy", JSON_INVALID_STRING);
    if (tmp_s == JSON_INVALID_STRING) {
        Shell_ExitSystem("'savegame_fmt_legacy' must be a string");
    }
    gf->savegame_fmt_legacy = Memory_DupStr(tmp_s);

    tmp_s = JSON_ObjectGetString(obj, "savegame_fmt_bson", JSON_INVALID_STRING);
    if (tmp_s == JSON_INVALID_STRING) {
        Shell_ExitSystem("'savegame_fmt_bson' must be a string");
    }
    gf->savegame_fmt_bson = Memory_DupStr(tmp_s);

    tmp_d = JSON_ObjectGetDouble(obj, "demo_delay", -1.0);
    if (tmp_d < 0.0) {
        Shell_ExitSystem("'demo_delay' must be a positive number");
    }
    gf->demo_delay = tmp_d;

    gf->settings = m_DefaultSettings;
    M_LoadSettings(obj, &gf->settings);

    tmp_arr = JSON_ObjectGetArray(obj, "injections");
    if (tmp_arr) {
        gf->injections.count = tmp_arr->length;
        gf->injections.data_paths =
            Memory_Alloc(sizeof(char *) * tmp_arr->length);
        for (size_t i = 0; i < tmp_arr->length; i++) {
            const char *const str = JSON_ArrayGetString(tmp_arr, i, NULL);
            gf->injections.data_paths[i] = Memory_DupStr(str);
        }
    } else {
        gf->injections.count = 0;
    }

    gf->enable_tr2_item_drops =
        JSON_ObjectGetBool(obj, "enable_tr2_item_drops", false);
    gf->convert_dropped_guns =
        JSON_ObjectGetBool(obj, "convert_dropped_guns", false);
    gf->enable_killer_pushblocks =
        JSON_ObjectGetBool(obj, "enable_killer_pushblocks", true);
}

void GF_Load(const char *const path)
{
    GF_Shutdown();

    char *script_data = NULL;
    if (!File_Load(path, &script_data, NULL)) {
        Shell_ExitSystem("Failed to open script file");
    }

    JSON_PARSE_RESULT parse_result;
    JSON_VALUE *const root = JSON_ParseEx(
        script_data, strlen(script_data), JSON_PARSE_FLAGS_ALLOW_JSON5, NULL,
        NULL, &parse_result);
    if (root == NULL) {
        Shell_ExitSystemFmt(
            "Failed to parse script file: %s in line %d, char %d",
            JSON_GetErrorDescription(parse_result.error),
            parse_result.error_line_no, parse_result.error_row_no, script_data);
    }
    JSON_OBJECT *const root_obj = JSON_ValueAsObject(root);

    GAME_FLOW *const gf = &g_GameFlow;
    M_LoadRoot(root_obj, gf);
    M_LoadLevels(root_obj, gf);
    M_LoadFMVs(root_obj, gf);

    if (root != NULL) {
        JSON_ValueFree(root);
    }
    Memory_FreePointer(&script_data);
}
