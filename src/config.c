#include <stdio.h>
#include <string.h>
#include "json_utils.h"
#include "config.h"

static int8_t ReadBarShowingMode(json_value* root, const char* name)
{
    const char* value_str = JSONGetStringValue(root, name);
    if (!value_str) {
        return T1M_BSM_DEFAULT;
    } else if (!strcmp(value_str, "flashing")) {
        return T1M_BSM_FLASHING;
    } else if (!strcmp(value_str, "always")) {
        return T1M_BSM_ALWAYS;
    }
    return T1M_BSM_DEFAULT;
}

static int8_t
ReadBarLocationConfig(json_value* root, const char* name, int8_t default_value)
{
    const char* value_str = JSONGetStringValue(root, name);
    if (!value_str) {
        return default_value;
    } else if (!strcmp(value_str, "top-left")) {
        return T1M_BL_VTOP | T1M_BL_HLEFT;
    } else if (!strcmp(value_str, "top-center")) {
        return T1M_BL_VTOP | T1M_BL_HCENTER;
    } else if (!strcmp(value_str, "top-right")) {
        return T1M_BL_VTOP | T1M_BL_HRIGHT;
    } else if (!strcmp(value_str, "bottom-left")) {
        return T1M_BL_VBOTTOM | T1M_BL_HLEFT;
    } else if (!strcmp(value_str, "bottom-center")) {
        return T1M_BL_VBOTTOM | T1M_BL_HCENTER;
    } else if (!strcmp(value_str, "bottom-right")) {
        return T1M_BL_VBOTTOM | T1M_BL_HRIGHT;
    }
    return default_value;
}

int T1MReadConfig()
{
    FILE* fp = fopen("Tomb1Main.json", "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    int cfg_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* cfg_data = malloc(cfg_size);
    if (!cfg_data) {
        fclose(fp);
        return 0;
    }
    fread(cfg_data, 1, cfg_size, fp);
    fclose(fp);

    json_value* json = json_parse((const json_char*)cfg_data, cfg_size);

    T1MConfig.disable_healing_between_levels =
        JSONGetBooleanValue(json, "disable_healing_between_levels");
    T1MConfig.disable_medpacks = JSONGetBooleanValue(json, "disable_medpacks");
    T1MConfig.disable_magnums = JSONGetBooleanValue(json, "disable_magnums");
    T1MConfig.disable_uzis = JSONGetBooleanValue(json, "disable_uzis");
    T1MConfig.disable_shotgun = JSONGetBooleanValue(json, "disable_shotgun");
    T1MConfig.enable_red_healthbar =
        JSONGetBooleanValue(json, "enable_red_healthbar");
    T1MConfig.enable_enemy_healthbar =
        JSONGetBooleanValue(json, "enable_enemy_healthbar");
    T1MConfig.enable_enhanced_look =
        JSONGetBooleanValue(json, "enable_enhanced_look");
    T1MConfig.enable_enhanced_ui =
        JSONGetBooleanValue(json, "enable_enhanced_ui");
    T1MConfig.enable_shotgun_flash =
        JSONGetBooleanValue(json, "enable_shotgun_flash");
    T1MConfig.enable_cheats = JSONGetBooleanValue(json, "enable_cheats");

    T1MConfig.healthbar_showing_mode =
        ReadBarShowingMode(json, "healthbar_showing_mode");

    T1MConfig.healthbar_location = ReadBarLocationConfig(
        json, "healthbar_location", T1M_BL_VTOP | T1M_BL_HLEFT);
    T1MConfig.airbar_location = ReadBarLocationConfig(
        json, "airbar_location", T1M_BL_VTOP | T1M_BL_HRIGHT);
    T1MConfig.enemy_healthbar_location = ReadBarLocationConfig(
        json, "enemy_healthbar_location", T1M_BL_VBOTTOM | T1M_BL_HLEFT);

    T1MConfig.enable_numeric_keys =
        JSONGetBooleanValue(json, "enable_numeric_keys");
    T1MConfig.fix_end_of_level_freeze =
        JSONGetBooleanValue(json, "fix_end_of_level_freeze");
    T1MConfig.fix_tihocan_secret_sound =
        JSONGetBooleanValue(json, "fix_tihocan_secret_sound");
    T1MConfig.fix_pyramid_secret_trigger =
        JSONGetBooleanValue(json, "fix_pyramid_secret_trigger");
    T1MConfig.fix_hardcoded_secret_counts =
        JSONGetBooleanValue(json, "fix_hardcoded_secret_counts");

    json_value_free(json);
    free(cfg_data);
    return 1;
}
