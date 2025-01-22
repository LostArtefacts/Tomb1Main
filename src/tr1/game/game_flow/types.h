#pragma once

#include "./enum.h"
#include "global/types.h"

#include <libtrx/game/game_flow/types.h>
#include <libtrx/game/objects/types.h>

// ----------------------------------------------------------------------------
// Sequencer structures
// ----------------------------------------------------------------------------

typedef struct {
    GAME_FLOW_SEQUENCE_EVENT_TYPE type;
    void *data;
} GAME_FLOW_SEQUENCE_EVENT;

typedef struct {
    int32_t length;
    GAME_FLOW_SEQUENCE_EVENT *events;
} GAME_FLOW_SEQUENCE;

// Concrete events data

typedef struct {
    char *path;
    double display_time;
} GAME_FLOW_DISPLAY_PICTURE_DATA;

typedef struct {
    GAME_OBJECT_ID object1_id;
    GAME_OBJECT_ID object2_id;
    int32_t mesh_num;
} GAME_FLOW_MESH_SWAP_DATA;

typedef struct {
    GAME_OBJECT_ID object_id;
    int quantity;
} GAME_FLOW_GIVE_ITEM_DATA;

typedef struct {
    int32_t enemy_num;
    int32_t count;
    int16_t *object_ids;
} GAME_FLOW_DROP_ITEM_DATA;

// ----------------------------------------------------------------------------
// Game flow structures
// ----------------------------------------------------------------------------

typedef struct {
    GAME_FLOW_LEVEL_TYPE level_type;
    int16_t music;
    char *title;
    char *path;
    int8_t demo;
    GAME_FLOW_SEQUENCE sequence;
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
