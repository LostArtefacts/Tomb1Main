#include "game/game_flow/reader.h"

#include "game/game_flow/common.h"
#include "game/game_flow/types.h"
#include "game/game_flow/vars.h"
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

static bool M_LoadScriptMeta(JSON_OBJECT *obj);
static bool M_IsLegacySequence(const char *type_str);
static bool M_LoadLevelSequence(JSON_OBJECT *obj, int32_t level_num);
static bool M_LoadScriptLevels(JSON_OBJECT *obj);

static bool M_LoadScriptMeta(JSON_OBJECT *obj)
{
    const char *tmp_s;
    int tmp_i;
    double tmp_d;
    JSON_ARRAY *tmp_arr;

    tmp_s = JSON_ObjectGetString(obj, "main_menu_picture", JSON_INVALID_STRING);
    if (tmp_s == JSON_INVALID_STRING) {
        LOG_ERROR("'main_menu_picture' must be a string");
        return false;
    }
    g_GameFlow.main_menu_background_path = Memory_DupStr(tmp_s);

    tmp_s =
        JSON_ObjectGetString(obj, "savegame_fmt_legacy", JSON_INVALID_STRING);
    if (tmp_s == JSON_INVALID_STRING) {
        LOG_ERROR("'savegame_fmt_legacy' must be a string");
        return false;
    }
    g_GameFlow.savegame_fmt_legacy = Memory_DupStr(tmp_s);

    tmp_s = JSON_ObjectGetString(obj, "savegame_fmt_bson", JSON_INVALID_STRING);
    if (tmp_s == JSON_INVALID_STRING) {
        LOG_ERROR("'savegame_fmt_bson' must be a string");
        return false;
    }
    g_GameFlow.savegame_fmt_bson = Memory_DupStr(tmp_s);

    tmp_d = JSON_ObjectGetDouble(obj, "demo_delay", -1.0);
    if (tmp_d < 0.0) {
        LOG_ERROR("'demo_delay' must be a positive number");
        return false;
    }
    g_GameFlow.demo_delay = tmp_d;

    tmp_arr = JSON_ObjectGetArray(obj, "water_color");
    g_GameFlow.water_color.r = 0.6;
    g_GameFlow.water_color.g = 0.7;
    g_GameFlow.water_color.b = 1.0;
    if (tmp_arr) {
        g_GameFlow.water_color.r =
            JSON_ArrayGetDouble(tmp_arr, 0, g_GameFlow.water_color.r);
        g_GameFlow.water_color.g =
            JSON_ArrayGetDouble(tmp_arr, 1, g_GameFlow.water_color.g);
        g_GameFlow.water_color.b =
            JSON_ArrayGetDouble(tmp_arr, 2, g_GameFlow.water_color.b);
    }

    if (JSON_ObjectGetValue(obj, "draw_distance_fade")) {
        double value = JSON_ObjectGetDouble(
            obj, "draw_distance_fade", JSON_INVALID_NUMBER);
        if (value == JSON_INVALID_NUMBER) {
            LOG_ERROR("'draw_distance_fade' must be a number");
            return false;
        }
        g_GameFlow.draw_distance_fade = value;
    } else {
        g_GameFlow.draw_distance_fade = 12.0f;
    }

    if (JSON_ObjectGetValue(obj, "draw_distance_max")) {
        double value =
            JSON_ObjectGetDouble(obj, "draw_distance_max", JSON_INVALID_NUMBER);
        if (value == JSON_INVALID_NUMBER) {
            LOG_ERROR("'draw_distance_max' must be a number");
            return false;
        }
        g_GameFlow.draw_distance_max = value;
    } else {
        g_GameFlow.draw_distance_max = 20.0f;
    }

    tmp_arr = JSON_ObjectGetArray(obj, "injections");
    if (tmp_arr) {
        g_GameFlow.injections.length = tmp_arr->length;
        g_GameFlow.injections.data_paths =
            Memory_Alloc(sizeof(char *) * tmp_arr->length);
        for (size_t i = 0; i < tmp_arr->length; i++) {
            const char *const str = JSON_ArrayGetString(tmp_arr, i, NULL);
            g_GameFlow.injections.data_paths[i] = Memory_DupStr(str);
        }
    } else {
        g_GameFlow.injections.length = 0;
    }

    g_GameFlow.enable_tr2_item_drops =
        JSON_ObjectGetBool(obj, "enable_tr2_item_drops", false);
    g_GameFlow.convert_dropped_guns =
        JSON_ObjectGetBool(obj, "convert_dropped_guns", false);
    g_GameFlow.enable_killer_pushblocks =
        JSON_ObjectGetBool(obj, "enable_killer_pushblocks", true);

    return true;
}

static bool M_IsLegacySequence(const char *type_str)
{
    return !strcmp(type_str, "fix_pyramid_secret")
        || !strcmp(type_str, "stop_cine");
}

static bool M_LoadLevelSequence(JSON_OBJECT *obj, int32_t level_num)
{
    JSON_ARRAY *jseq_arr = JSON_ObjectGetArray(obj, "sequence");
    if (!jseq_arr) {
        LOG_ERROR("level %d: 'sequence' must be a list", level_num);
        return false;
    }

    JSON_ARRAY_ELEMENT *jseq_elem = jseq_arr->start;

    GAME_FLOW_SEQUENCE *sequence = &g_GameFlow.levels[level_num].sequence;
    sequence->length = jseq_arr->length;
    sequence->events =
        Memory_Alloc(sizeof(GAME_FLOW_SEQUENCE_EVENT) * jseq_arr->length);

    GAME_FLOW_SEQUENCE_EVENT *event = sequence->events;
    int32_t i = 0;
    while (jseq_elem) {
        JSON_OBJECT *jseq_obj = JSON_ValueAsObject(jseq_elem->value);
        if (!jseq_obj) {
            LOG_ERROR("level %d: 'sequence' elements must be dictionaries");
            return false;
        }

        const char *type_str =
            JSON_ObjectGetString(jseq_obj, "type", JSON_INVALID_STRING);
        if (type_str == JSON_INVALID_STRING) {
            LOG_ERROR("level %d: sequence 'type' must be a string", level_num);
            return false;
        }

        event->type = ENUM_MAP_GET(GAME_FLOW_SEQUENCE_EVENT_TYPE, type_str, -1);

        switch (event->type) {
        case GFS_START_GAME:
        case GFS_STOP_GAME:
        case GFS_LOOP_GAME:
        case GFS_START_CINE:
        case GFS_LOOP_CINE:
            event->data = (void *)(intptr_t)level_num;
            break;

        case GFS_PLAY_FMV: {
            const char *tmp_s =
                JSON_ObjectGetString(jseq_obj, "fmv_path", JSON_INVALID_STRING);
            if (tmp_s == JSON_INVALID_STRING) {
                LOG_ERROR(
                    "level %d, sequence %s: 'fmv_path' must be a string",
                    level_num, type_str);
                return false;
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
                LOG_ERROR(
                    "level %d, sequence %s: 'picture_path' must be a string",
                    level_num, type_str);
                return false;
            }
            data->path = Memory_DupStr(tmp_s);

            double tmp_d = JSON_ObjectGetDouble(jseq_obj, "display_time", -1.0);
            if (tmp_d < 0.0) {
                LOG_ERROR(
                    "level %d, sequence %s: 'display_time' must be a positive "
                    "number",
                    level_num, type_str);
                return false;
            }
            data->display_time = tmp_d;
            event->data = data;
            break;
        }

        case GFS_LEVEL_STATS: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "level_id", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'level_id' must be a number",
                    level_num, type_str);
                return false;
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
                LOG_ERROR(
                    "level %d, sequence %s: 'picture_path' must be a string",
                    level_num, type_str);
                return false;
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
                LOG_ERROR(
                    "level %d, sequence %s: 'level_id' must be a number",
                    level_num, type_str);
                return false;
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        case GFS_SET_CAM_X:
        case GFS_SET_CAM_Y:
        case GFS_SET_CAM_Z:
        case GFS_SET_CAM_ANGLE: {
            int tmp = JSON_ObjectGetInt(jseq_obj, "value", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'value' must be a number",
                    level_num, type_str);
                return false;
            }
            event->data = (void *)(intptr_t)tmp;
            break;
        }

        case GFS_FLIP_MAP:
        case GFS_REMOVE_GUNS:
        case GFS_REMOVE_SCIONS:
        case GFS_REMOVE_AMMO:
        case GFS_REMOVE_MEDIPACKS:
            break;

        case GFS_GIVE_ITEM: {
            GAME_FLOW_GIVE_ITEM_DATA *give_item_data =
                Memory_Alloc(sizeof(GAME_FLOW_GIVE_ITEM_DATA));

            give_item_data->object_id =
                JSON_ObjectGetInt(jseq_obj, "object_id", JSON_INVALID_NUMBER);
            if (give_item_data->object_id == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'object_id' must be a number",
                    level_num, type_str);
                return false;
            }

            give_item_data->quantity =
                JSON_ObjectGetInt(jseq_obj, "quantity", 1);

            event->data = give_item_data;
            break;
        }

        case GFS_PLAY_SYNCED_AUDIO: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "audio_id", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'audio_id' must be a number",
                    level_num, type_str);
                return false;
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
                LOG_ERROR(
                    "level %d, sequence %s: 'object1_id' must be a number",
                    level_num, type_str);
                return false;
            }

            swap_data->object2_id =
                JSON_ObjectGetInt(jseq_obj, "object2_id", JSON_INVALID_NUMBER);
            if (swap_data->object2_id == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'object2_id' must be a number",
                    level_num, type_str);
                return false;
            }

            swap_data->mesh_num =
                JSON_ObjectGetInt(jseq_obj, "mesh_id", JSON_INVALID_NUMBER);
            if (swap_data->mesh_num == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'mesh_id' must be a number",
                    level_num, type_str);
                return false;
            }

            event->data = swap_data;
            break;
        }

        case GFS_SETUP_BACON_LARA: {
            int tmp =
                JSON_ObjectGetInt(jseq_obj, "anchor_room", JSON_INVALID_NUMBER);
            if (tmp == JSON_INVALID_NUMBER) {
                LOG_ERROR(
                    "level %d, sequence %s: 'anchor_room' must be a number",
                    level_num, type_str);
                return false;
            }
            if (tmp < 0) {
                LOG_ERROR(
                    "level %d, sequence %s: 'anchor_room' must be >= 0",
                    level_num, type_str);
                return false;
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
                LOG_ERROR("unknown sequence type %s", type_str);
                return false;
            }
            break;
        }

        jseq_elem = jseq_elem->next;
        i++;
        event++;
    }

    return true;
}

static bool M_LoadScriptLevels(JSON_OBJECT *obj)
{
    JSON_ARRAY *jlvl_arr = JSON_ObjectGetArray(obj, "levels");
    if (!jlvl_arr) {
        LOG_ERROR("'levels' must be a list");
        return false;
    }

    int32_t level_count = jlvl_arr->length;

    g_GameFlow.levels = Memory_Alloc(sizeof(GAME_FLOW_LEVEL) * level_count);
    g_GameInfo.current = Memory_Alloc(sizeof(RESUME_INFO) * level_count);

    JSON_ARRAY_ELEMENT *jlvl_elem = jlvl_arr->start;
    int level_num = 0;

    g_GameFlow.has_demo = 0;
    g_GameFlow.gym_level_num = -1;
    g_GameFlow.first_level_num = -1;
    g_GameFlow.last_level_num = -1;
    g_GameFlow.title_level_num = -1;
    g_GameFlow.level_count = jlvl_arr->length;

    GAME_FLOW_LEVEL *cur = &g_GameFlow.levels[0];
    while (jlvl_elem) {
        JSON_OBJECT *jlvl_obj = JSON_ValueAsObject(jlvl_elem->value);
        if (!jlvl_obj) {
            LOG_ERROR("'levels' elements must be dictionaries");
            return false;
        }

        const char *tmp_s;
        int32_t tmp_i;
        JSON_ARRAY *tmp_arr;

        tmp_i = JSON_ObjectGetInt(jlvl_obj, "music", JSON_INVALID_NUMBER);
        if (tmp_i == JSON_INVALID_NUMBER) {
            LOG_ERROR("level %d: 'music' must be a number", level_num);
            return false;
        }
        cur->music = tmp_i;

        tmp_s = JSON_ObjectGetString(jlvl_obj, "file", JSON_INVALID_STRING);
        if (tmp_s == JSON_INVALID_STRING) {
            LOG_ERROR("level %d: 'file' must be a string", level_num);
            return false;
        }
        cur->path = Memory_DupStr(tmp_s);

        tmp_s = JSON_ObjectGetString(jlvl_obj, "type", JSON_INVALID_STRING);
        if (tmp_s == JSON_INVALID_STRING) {
            LOG_ERROR("level %d: 'type' must be a string", level_num);
            return false;
        }

        cur->level_type = ENUM_MAP_GET(GAME_FLOW_LEVEL_TYPE, tmp_s, -1);

        switch (cur->level_type) {
        case GFL_TITLE:
        case GFL_TITLE_DEMO_PC:
            if (g_GameFlow.title_level_num != -1) {
                LOG_ERROR(
                    "level %d: there can be only one title level", level_num);
                return false;
            }
            g_GameFlow.title_level_num = level_num;
            break;

        case GFL_GYM:
            if (g_GameFlow.gym_level_num != -1) {
                LOG_ERROR(
                    "level %d: there can be only one gym level", level_num);
                return false;
            }
            g_GameFlow.gym_level_num = level_num;
            break;

        case GFL_LEVEL_DEMO_PC:
        case GFL_NORMAL:
            if (g_GameFlow.first_level_num == -1) {
                g_GameFlow.first_level_num = level_num;
            }
            g_GameFlow.last_level_num = level_num;
            break;

        case GFL_BONUS:
        case GFL_CUTSCENE:
        case GFL_CURRENT:
            break;

        default:
            LOG_ERROR("level %d: unknown level type %s", level_num, tmp_s);
            return false;
        }

        tmp_i = JSON_ObjectGetBool(jlvl_obj, "demo", JSON_INVALID_BOOL);
        if (tmp_i != JSON_INVALID_BOOL) {
            cur->demo = tmp_i;
            g_GameFlow.has_demo |= tmp_i;
        } else {
            cur->demo = 0;
        }

        {
            double value = JSON_ObjectGetDouble(
                jlvl_obj, "draw_distance_fade", JSON_INVALID_NUMBER);
            if (value != JSON_INVALID_NUMBER) {
                cur->draw_distance_fade.override = true;
                cur->draw_distance_fade.value = value;
            } else {
                cur->draw_distance_fade.override = false;
            }
        }

        {
            double value = JSON_ObjectGetDouble(
                jlvl_obj, "draw_distance_max", JSON_INVALID_NUMBER);
            if (value != JSON_INVALID_NUMBER) {
                cur->draw_distance_max.override = true;
                cur->draw_distance_max.value = value;
            } else {
                cur->draw_distance_max.override = false;
            }
        }

        tmp_arr = JSON_ObjectGetArray(jlvl_obj, "water_color");
        if (tmp_arr) {
            cur->water_color.override = true;
            cur->water_color.value.r =
                JSON_ArrayGetDouble(tmp_arr, 0, g_GameFlow.water_color.r);
            cur->water_color.value.g =
                JSON_ArrayGetDouble(tmp_arr, 1, g_GameFlow.water_color.g);
            cur->water_color.value.b =
                JSON_ArrayGetDouble(tmp_arr, 2, g_GameFlow.water_color.b);
        } else {
            cur->water_color.override = false;
        }

        cur->unobtainable.pickups =
            JSON_ObjectGetInt(jlvl_obj, "unobtainable_pickups", 0);

        cur->unobtainable.kills =
            JSON_ObjectGetInt(jlvl_obj, "unobtainable_kills", 0);

        cur->unobtainable.secrets =
            JSON_ObjectGetInt(jlvl_obj, "unobtainable_secrets", 0);

        tmp_i = JSON_ObjectGetBool(jlvl_obj, "inherit_injections", 1);
        tmp_arr = JSON_ObjectGetArray(jlvl_obj, "injections");
        if (tmp_arr) {
            cur->injections.length = tmp_arr->length;
            if (tmp_i) {
                cur->injections.length += g_GameFlow.injections.length;
            }
            cur->injections.data_paths =
                Memory_Alloc(sizeof(char *) * cur->injections.length);

            int inj_base_index = 0;
            if (tmp_i) {
                for (int i = 0; i < g_GameFlow.injections.length; i++) {
                    cur->injections.data_paths[i] =
                        Memory_DupStr(g_GameFlow.injections.data_paths[i]);
                }
                inj_base_index = g_GameFlow.injections.length;
            }

            for (size_t i = 0; i < tmp_arr->length; i++) {
                const char *const str = JSON_ArrayGetString(tmp_arr, i, NULL);
                cur->injections.data_paths[inj_base_index + i] =
                    Memory_DupStr(str);
            }
        } else if (tmp_i) {
            cur->injections.length = g_GameFlow.injections.length;
            cur->injections.data_paths =
                Memory_Alloc(sizeof(char *) * cur->injections.length);
            for (int i = 0; i < g_GameFlow.injections.length; i++) {
                cur->injections.data_paths[i] =
                    Memory_DupStr(g_GameFlow.injections.data_paths[i]);
            }
        } else {
            cur->injections.length = 0;
        }

        tmp_i = JSON_ObjectGetInt(jlvl_obj, "lara_type", (int32_t)O_LARA);
        if (tmp_i < 0 || tmp_i >= O_NUMBER_OF) {
            LOG_ERROR(
                "level %d: 'lara_type' must be a valid game object id",
                level_num);
            return false;
        }
        cur->lara_type = (GAME_OBJECT_ID)tmp_i;

        tmp_arr = JSON_ObjectGetArray(jlvl_obj, "item_drops");
        cur->item_drops.count = 0;
        if (tmp_arr && g_GameFlow.enable_tr2_item_drops) {
            LOG_WARNING(
                "TR2 item drops are enabled: gameflow-defined drops for level "
                "%d will be ignored",
                level_num);
        } else if (tmp_arr) {
            cur->item_drops.count = (signed)tmp_arr->length;
            cur->item_drops.data = Memory_Alloc(
                sizeof(GAME_FLOW_DROP_ITEM_DATA) * (signed)tmp_arr->length);

            for (int i = 0; i < cur->item_drops.count; i++) {
                GAME_FLOW_DROP_ITEM_DATA *data = &cur->item_drops.data[i];
                JSON_OBJECT *jlvl_data = JSON_ArrayGetObject(tmp_arr, i);

                data->enemy_num = JSON_ObjectGetInt(
                    jlvl_data, "enemy_num", JSON_INVALID_NUMBER);
                if (data->enemy_num == JSON_INVALID_NUMBER) {
                    LOG_ERROR(
                        "level %d, item drop %d: 'enemy_num' must be a number",
                        level_num, i);
                    return false;
                }

                JSON_ARRAY *object_arr =
                    JSON_ObjectGetArray(jlvl_data, "object_ids");
                if (!object_arr) {
                    LOG_ERROR(
                        "level %d, item drop %d: 'object_ids' must be an array",
                        level_num, i);
                    return false;
                }

                data->count = (signed)object_arr->length;
                data->object_ids = Memory_Alloc(sizeof(int16_t) * data->count);
                for (int j = 0; j < data->count; j++) {
                    int id = JSON_ArrayGetInt(object_arr, j, -1);
                    if (id < 0 || id >= O_NUMBER_OF) {
                        LOG_ERROR(
                            "level %d, item drop %d, index %d: 'object_id' "
                            "must be a valid object id",
                            level_num, i, j);
                        return false;
                    }
                    data->object_ids[j] = (int16_t)id;
                }
            }
        }

        if (!M_LoadLevelSequence(jlvl_obj, level_num)) {
            return false;
        }

        jlvl_elem = jlvl_elem->next;
        level_num++;
        cur++;
    }

    if (g_GameFlow.title_level_num == -1) {
        LOG_ERROR("at least one level must be of title type");
        return false;
    }
    if (g_GameFlow.first_level_num == -1 || g_GameFlow.last_level_num == -1) {
        LOG_ERROR("at least one level must be of normal type");
        return false;
    }
    return true;
}

bool GF_Load(const char *file_name)
{
    GF_Shutdown();
    bool result = false;
    JSON_VALUE *root = NULL;
    char *script_data = NULL;

    if (!File_Load(file_name, &script_data, NULL)) {
        LOG_ERROR("failed to open script file");
        goto cleanup;
    }

    JSON_PARSE_RESULT parse_result;
    root = JSON_ParseEx(
        script_data, strlen(script_data), JSON_PARSE_FLAGS_ALLOW_JSON5, NULL,
        NULL, &parse_result);
    if (!root) {
        LOG_ERROR(
            "failed to parse script file: %s in line %d, char %d",
            JSON_GetErrorDescription(parse_result.error),
            parse_result.error_line_no, parse_result.error_row_no, script_data);
        goto cleanup;
    }

    JSON_OBJECT *root_obj = JSON_ValueAsObject(root);

    result = true;
    result &= M_LoadScriptMeta(root_obj);
    result &= M_LoadScriptLevels(root_obj);

cleanup:
    if (root) {
        JSON_ValueFree(root);
        root = NULL;
    }

    Memory_FreePointer(&script_data);
    return result;
}
