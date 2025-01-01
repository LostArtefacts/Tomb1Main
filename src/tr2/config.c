#include "config.h"

#include "game/clock.h"
#include "game/input.h"
#include "game/music.h"
#include "game/shell.h"
#include "game/sound.h"

#include <libtrx/config/file.h>
#include <libtrx/debug.h>
#include <libtrx/log.h>
#include <libtrx/utils.h>

#include <stdio.h>

// TODO: eliminate me
extern const CONFIG_OPTION g_ConfigOptionMap[];

static const char *m_ConfigPath = "cfg/TR2X.json5";

static void M_LoadInputConfig(JSON_OBJECT *root_obj);
static void M_LoadInputLayout(
    JSON_OBJECT *parent_obj, INPUT_BACKEND backend, INPUT_LAYOUT layout);
static void M_DumpInputConfig(JSON_OBJECT *root_obj);
static void M_DumpInputLayout(
    JSON_OBJECT *parent_obj, INPUT_BACKEND backend, INPUT_LAYOUT layout);

static void M_LoadInputConfig(JSON_OBJECT *const root_obj)
{
    JSON_OBJECT *const input_obj = JSON_ObjectGetObject(root_obj, "input");
    if (input_obj == NULL) {
        return;
    }

    JSON_OBJECT *const keyboard_obj =
        JSON_ObjectGetObject(input_obj, "keyboard");
    JSON_OBJECT *const controller_obj =
        JSON_ObjectGetObject(input_obj, "controller");
    for (INPUT_LAYOUT layout = INPUT_LAYOUT_CUSTOM_1;
         layout < INPUT_LAYOUT_NUMBER_OF; layout++) {
        if (keyboard_obj != NULL) {
            M_LoadInputLayout(keyboard_obj, INPUT_BACKEND_KEYBOARD, layout);
        }
        if (controller_obj != NULL) {
            M_LoadInputLayout(controller_obj, INPUT_BACKEND_CONTROLLER, layout);
        }
    }
}

static void M_LoadInputLayout(
    JSON_OBJECT *const parent_obj, const INPUT_BACKEND backend,
    const INPUT_LAYOUT layout)
{
    char layout_name[20];
    sprintf(layout_name, "layout_%d", layout);
    JSON_ARRAY *const arr = JSON_ObjectGetArray(parent_obj, layout_name);
    if (arr == NULL) {
        return;
    }

    for (size_t i = 0; i < arr->length; i++) {
        JSON_OBJECT *const bind_obj = JSON_ArrayGetObject(arr, i);
        ASSERT(bind_obj != NULL);
        Input_AssignFromJSONObject(backend, layout, bind_obj);
    }
}

static void M_DumpInputConfig(JSON_OBJECT *const root_obj)
{
    JSON_OBJECT *const input_obj = JSON_ObjectNew();
    JSON_OBJECT *const keyboard_obj = JSON_ObjectNew();
    JSON_OBJECT *const controller_obj = JSON_ObjectNew();
    JSON_ObjectAppendObject(root_obj, "input", input_obj);
    JSON_ObjectAppendObject(input_obj, "keyboard", keyboard_obj);
    JSON_ObjectAppendObject(input_obj, "controller", controller_obj);
    for (INPUT_LAYOUT layout = INPUT_LAYOUT_CUSTOM_1;
         layout < INPUT_LAYOUT_NUMBER_OF; layout++) {
        M_DumpInputLayout(keyboard_obj, INPUT_BACKEND_KEYBOARD, layout);
        M_DumpInputLayout(controller_obj, INPUT_BACKEND_CONTROLLER, layout);
    }
}

static void M_DumpInputLayout(
    JSON_OBJECT *const parent_obj, const INPUT_BACKEND backend,
    const INPUT_LAYOUT layout)
{
    JSON_ARRAY *const arr = JSON_ArrayNew();

    bool has_elements = false;
    for (INPUT_ROLE role = 0; role < INPUT_ROLE_NUMBER_OF; role++) {
        JSON_OBJECT *const bind_obj = JSON_ObjectNew();
        if (Input_AssignToJSONObject(backend, layout, bind_obj, role)) {
            has_elements = true;
            JSON_ArrayAppendObject(arr, bind_obj);
        } else {
            JSON_ObjectFree(bind_obj);
        }
    }

    if (has_elements) {
        char layout_name[20];
        sprintf(layout_name, "layout_%d", layout);
        JSON_ObjectAppendArray(parent_obj, layout_name, arr);
    } else {
        JSON_ArrayFree(arr);
    }
}

const char *Config_GetPath(const CONFIG_FILE_TYPE file_type)
{
    return file_type == CFT_DEFAULT ? m_ConfigPath : Shell_GetGameflowPath();
}

void Config_LoadFromJSON(JSON_OBJECT *root_obj)
{
    ConfigFile_LoadOptions(root_obj, g_ConfigOptionMap);
    M_LoadInputConfig(root_obj);
    g_Config.loaded = true;
    g_SavedConfig = g_Config;
}

void Config_DumpToJSON(JSON_OBJECT *root_obj)
{
    ConfigFile_DumpOptions(root_obj, g_ConfigOptionMap);
    M_DumpInputConfig(root_obj);
}

void Config_Sanitize(void)
{
    CLAMP(
        g_Config.gameplay.turbo_speed, CLOCK_TURBO_SPEED_MIN,
        CLOCK_TURBO_SPEED_MAX);
    CLAMP(g_Config.rendering.scaler, 1, 4);

    if (g_Config.rendering.render_mode != RM_HARDWARE
        && g_Config.rendering.render_mode != RM_SOFTWARE) {
        g_Config.rendering.render_mode = RM_SOFTWARE;
    }
    if (g_Config.rendering.aspect_mode != AM_ANY
        && g_Config.rendering.aspect_mode != AM_16_9) {
        g_Config.rendering.aspect_mode = AM_4_3;
    }
    if (g_Config.rendering.texel_adjust_mode != TAM_DISABLED
        && g_Config.rendering.texel_adjust_mode != TAM_BILINEAR_ONLY) {
        g_Config.rendering.texel_adjust_mode = TAM_ALWAYS;
    }
    CLAMP(g_Config.rendering.nearest_adjustment, 0, 256);
    CLAMP(g_Config.rendering.linear_adjustment, 0, 256);

    CLAMP(g_Config.visuals.fov, 30, 150);
    CLAMP(g_Config.ui.bar_scale, 0.5, 2.0);
    CLAMP(g_Config.ui.text_scale, 0.5, 2.0);
}

void Config_ApplyChanges(void)
{
    Sound_SetMasterVolume(g_Config.audio.sound_volume);
    Music_SetVolume(g_Config.audio.music_volume);

    g_SavedConfig = g_Config;
}

const CONFIG_OPTION *Config_GetOptionMap(void)
{
    return g_ConfigOptionMap;
}
