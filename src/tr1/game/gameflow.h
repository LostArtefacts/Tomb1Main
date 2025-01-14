#pragma once

#include "global/types.h"

#include <libtrx/game/inventory_ring/types.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char *key;
    const char *value;
} GAME_FLOW_STRING_ENTRY;

typedef struct {
    GAME_FLOW_SEQUENCE_TYPE type;
    void *data;
} GAME_FLOW_SEQUENCE;

typedef struct {
    int32_t enemy_num;
    int32_t count;
    int16_t *object_ids;
} GAME_FLOW_DROP_ITEM_DATA;

typedef struct {
    GAME_FLOW_LEVEL_TYPE level_type;
    int16_t music;
    char *level_title;
    char *level_file;
    GAME_FLOW_STRING_ENTRY *object_strings;
    GAME_FLOW_STRING_ENTRY *examine_strings;
    int8_t demo;
    GAME_FLOW_SEQUENCE *sequence;
    struct {
        bool override;
        RGB_F value;
    } water_color;
    struct {
        bool override;
        float value;
    } draw_distance_fade;
    struct {
        bool override;
        float value;
    } draw_distance_max;
    struct {
        uint32_t pickups;
        uint32_t kills;
        uint32_t secrets;
    } unobtainable;
    struct {
        int length;
        char **data_paths;
    } injections;
    struct {
        int count;
        GAME_FLOW_DROP_ITEM_DATA *data;
    } item_drops;
    GAME_OBJECT_ID lara_type;
} GAME_FLOW_LEVEL;

typedef struct {
    char *main_menu_background_path;
    int32_t gym_level_num;
    int32_t first_level_num;
    int32_t last_level_num;
    int32_t title_level_num;
    int32_t level_count;
    char *savegame_fmt_legacy;
    char *savegame_fmt_bson;
    int8_t has_demo;
    double demo_delay;
    GAME_FLOW_LEVEL *levels;
    RGB_F water_color;
    float draw_distance_fade;
    float draw_distance_max;
    struct {
        int length;
        char **data_paths;
    } injections;
    bool enable_tr2_item_drops;
    bool convert_dropped_guns;
    bool enable_killer_pushblocks;
} GAME_FLOW;

extern GAME_FLOW g_GameFlow;

GAME_FLOW_COMMAND
GameFlow_InterpretSequence(int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND
GameFlow_StorySoFar(int32_t level_num, int32_t savegame_level);
GAME_FLOW_COMMAND GameFlow_PlayAvailableStory(int32_t slot_num);

GAME_FLOW_COMMAND GF_LoadLevel(
    int32_t level_num, GAME_FLOW_LEVEL_TYPE level_type);
GAME_FLOW_COMMAND GF_PlayDemo(int32_t demo_num);
GAME_FLOW_COMMAND GF_PlayCutscene(int32_t level_num);
GAME_FLOW_COMMAND GF_PauseGame(void);
GAME_FLOW_COMMAND GF_EnterPhotoMode(void);
GAME_FLOW_COMMAND GF_ShowInventory(INVENTORY_MODE inv_mode);
GAME_FLOW_COMMAND GF_ShowInventoryKeys(GAME_OBJECT_ID receptacle_type_id);

bool GameFlow_LoadFromFile(const char *file_name);
void GameFlow_Shutdown(void);

void GameFlow_LoadStrings(int32_t level_num);
