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

typedef struct {
    const char *str;
    const int32_t val;
} STRING_TO_ENUM_TYPE;

static GAME_FLOW_LEVEL_SETTINGS m_DefaultSettings = {
    .water_color = { .r = 0.6, .g = 0.7, .b = 1.0 },
    .draw_distance_fade = 12.0f,
    .draw_distance_max = 20.0f,
};

static bool M_IsLegacySequence(const char *type_str);
static void M_LoadSettings(
    JSON_OBJECT *obj, GAME_FLOW_LEVEL_SETTINGS *settings);
static void M_LoadLevelSequence(
    JSON_OBJECT *obj, GAME_FLOW *gf, int32_t level_num);
static void M_LoadLevels(JSON_OBJECT *obj, GAME_FLOW *gf);
static void M_LoadRoot(JSON_OBJECT *obj, GAME_FLOW *gf);

static bool M_IsLegacySequence(const char *const type_str)
{
    return strcmp(type_str, "fix_pyramid_secret") == 0
        || strcmp(type_str, "stop_cine") == 0;
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

static void M_LoadLevelSequence(
    JSON_OBJECT *const obj, GAME_FLOW *const gf, const int32_t level_num)
{
    JSON_ARRAY *jseq_arr = JSON_ObjectGetArray(obj, "sequence");
    if (!jseq_arr) {
        Shell_ExitSystemFmt("level %d: 'sequence' must be a list", level_num);
    }

    JSON_ARRAY_ELEMENT *jseq_elem = jseq_arr->start;

    GAME_FLOW_SEQUENCE *sequence = &gf->levels[level_num].sequence;
    sequence->length = jseq_arr->length;
    sequence->events =
        Memory_Alloc(sizeof(GAME_FLOW_SEQUENCE_EVENT) * jseq_arr->length);

    GAME_FLOW_SEQUENCE_EVENT *event = sequence->events;
    int32_t i = 0;
    while (jseq_elem) {
        JSON_OBJECT *jseq_obj = JSON_ValueAsObject(jseq_elem->value);
        if (!jseq_obj) {
            Shell_ExitSystemFmt(
                "level %d: 'sequence' elements must be dictionaries");
        }

        const char *type_str =
            JSON_ObjectGetString(jseq_obj, "type", JSON_INVALID_STRING);
        if (type_str == JSON_INVALID_STRING) {
            Shell_ExitSystemFmt(
                "level %d: sequence 'type' must be a string", level_num);
        }

        event->type = ENUM_MAP_GET(GAME_FLOW_SEQUENCE_EVENT_TYPE, type_str, -1);

        switch (event->type) {
        case GFS_LOAD_LEVEL:
        case GFS_PLAY_LEVEL:
            event->data = (void *)(intptr_t)level_num;
            break;

        case GFS_PLAY_FMV: {
            const char *tmp_s =
                JSON_ObjectGetString(jseq_obj, "fmv_path", JSON_INVALID_STRING);
            if (tmp_s == JSON_INVALID_STRING) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'fmv_path' must be a string",
                    level_num, type_str);
            }
            event->data = Memory_DupStr(tmp_s);
            break;
        }

        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE: {
            GAME_FLOW_DISPLAY_PICTURE_DATA *data =
                Memory_Alloc(sizeof(GAME_FLOW_DISPLAY_PICTURE_DATA));

            const char *tmp_s = JSON_ObjectGetString(
                jseq_obj, "picture_path", JSON_INVALID_STRING);
            if (tmp_s == JSON_INVALID_STRING) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'picture_path' must be a string",
                    level_num, type_str);
            }
            data->path = Memory_DupStr(tmp_s);

            double tmp_d = JSON_ObjectGetDouble(jseq_obj, "display_time", -1.0);
            if (tmp_d < 0.0) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'display_time' must be a positive "
                    "number",
                    level_num, type_str);
            }
            data->display_time = tmp_d;
            event->data = data;
            break;
        }

        case GFS_LEVEL_STATS: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "level_id", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'level_id' must be a number",
                    level_num, type_str);
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        case GFS_TOTAL_STATS: {
            GAME_FLOW_DISPLAY_PICTURE_DATA *data =
                Memory_Alloc(sizeof(GAME_FLOW_DISPLAY_PICTURE_DATA));

            const char *tmp_s = JSON_ObjectGetString(
                jseq_obj, "picture_path", JSON_INVALID_STRING);
            if (tmp_s == JSON_INVALID_STRING) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'picture_path' must be a string",
                    level_num, type_str);
            }
            data->path = Memory_DupStr(tmp_s);
            data->display_time = 0;
            event->data = data;
            break;
        }

        case GFS_EXIT_TO_TITLE:
            break;

        case GFS_EXIT_TO_LEVEL:
        case GFS_EXIT_TO_CINE: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "level_id", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'level_id' must be a number",
                    level_num, type_str);
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        case GFS_SET_CAMERA_ANGLE: {
            int tmp = JSON_ObjectGetInt(jseq_obj, "value", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'value' must be a number",
                    level_num, type_str);
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        case GFS_FLIP_MAP:
        case GFS_REMOVE_WEAPONS:
        case GFS_REMOVE_SCIONS:
        case GFS_REMOVE_AMMO:
        case GFS_REMOVE_MEDIPACKS:
            break;

        case GFS_ADD_ITEM: {
            GAME_FLOW_ADD_ITEM_DATA *add_item_data =
                Memory_Alloc(sizeof(GAME_FLOW_ADD_ITEM_DATA));

            add_item_data->object_id =
                JSON_ObjectGetInt(jseq_obj, "object_id", JSON_INVALID_NUMBER);
            if (add_item_data->object_id == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'object_id' must be a number",
                    level_num, type_str);
            }

            add_item_data->quantity =
                JSON_ObjectGetInt(jseq_obj, "quantity", 1);

            event->data = add_item_data;
            break;
        }

        case GFS_PLAY_SYNCED_AUDIO: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "audio_id", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'audio_id' must be a number",
                    level_num, type_str);
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        case GFS_MESH_SWAP: {
            GAME_FLOW_MESH_SWAP_DATA *swap_data =
                Memory_Alloc(sizeof(GAME_FLOW_MESH_SWAP_DATA));

            swap_data->object1_id =
                JSON_ObjectGetInt(jseq_obj, "object1_id", JSON_INVALID_NUMBER);
            if (swap_data->object1_id == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'object1_id' must be a number",
                    level_num, type_str);
            }

            swap_data->object2_id =
                JSON_ObjectGetInt(jseq_obj, "object2_id", JSON_INVALID_NUMBER);
            if (swap_data->object2_id == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'object2_id' must be a number",
                    level_num, type_str);
            }

            swap_data->mesh_num =
                JSON_ObjectGetInt(jseq_obj, "mesh_id", JSON_INVALID_NUMBER);
            if (swap_data->mesh_num == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'mesh_id' must be a number",
                    level_num, type_str);
            }

            event->data = swap_data;
            break;
        }

        case GFS_SETUP_BACON_LARA: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "anchor_room", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'anchor_room' must be a number",
                    level_num, type_str);
            }
            if (tmp < 0) {
                Shell_ExitSystemFmt(
                    "level %d, sequence %s: 'anchor_room' must be >= 0",
                    level_num, type_str);
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        default:
            if (M_IsLegacySequence(type_str)) {
                event->type = GFS_LEGACY;
                LOG_WARNING(
                    "level %d, sequence %s: legacy type ignored", level_num,
                    type_str);

            } else {
                Shell_ExitSystemFmt("unknown sequence type %s", type_str);
            }
            break;
        }

        jseq_elem = jseq_elem->next;
        i++;
        event++;
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

    if (root != NULL) {
        JSON_ValueFree(root);
    }
    Memory_FreePointer(&script_data);
}
