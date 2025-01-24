#pragma once

#include "../music/ids.h"
#include "../objects/types.h"
#include "./enum.h"

#include <stdint.h>

typedef struct GAME_FLOW_COMMAND {
    GAME_FLOW_ACTION action;
    int32_t param;
} GAME_FLOW_COMMAND;

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

// ----------------------------------------------------------------------------
// Game flow level structures
// ----------------------------------------------------------------------------

typedef struct {
    int32_t count;
    char **data_paths;
} INJECTION_DATA;

typedef struct {
    const char *path;
} GAME_FLOW_FMV;

#if TR_VERSION == 1
typedef struct {
    RGB_F water_color;
    float draw_distance_fade;
    float draw_distance_max;
} GAME_FLOW_LEVEL_SETTINGS;

typedef struct {
    int32_t enemy_num;
    int32_t count;
    int16_t *object_ids;
} GAME_FLOW_DROP_ITEM_DATA;
#endif

typedef struct {
    int32_t num;
    GAME_FLOW_LEVEL_TYPE type;
#if TR_VERSION == 1
    char *path;
#elif TR_VERSION == 2
    const char *path;
#endif
    char *title;

    MUSIC_TRACK_ID music_track;
    GAME_FLOW_SEQUENCE sequence;
    INJECTION_DATA injections;

#if TR_VERSION == 1
    GAME_FLOW_LEVEL_SETTINGS settings;

    bool demo;

    struct {
        uint32_t pickups;
        uint32_t kills;
        uint32_t secrets;
    } unobtainable;

    struct {
        int count;
        GAME_FLOW_DROP_ITEM_DATA *data;
    } item_drops;

    GAME_OBJECT_ID lara_type;
#endif
} GAME_FLOW_LEVEL;

// ----------------------------------------------------------------------------
// Game flow structures
// ----------------------------------------------------------------------------

typedef struct {
    // levels
    struct {
        int32_t level_count;
        GAME_FLOW_LEVEL *levels;
    };

#if TR_VERSION == 1
    int32_t gym_level_num;
    int32_t first_level_num;
    int32_t last_level_num;
    int32_t title_level_num;
#elif TR_VERSION == 2
    GAME_FLOW_LEVEL *title_level;

    // cutscenes
    struct {
        int32_t cutscene_count;
        GAME_FLOW_LEVEL *cutscenes;
    };

    // demos
    struct {
        int32_t demo_count;
        GAME_FLOW_LEVEL *demos;
    };
#endif

    // FMVs
    struct {
        int32_t fmv_count;
        GAME_FLOW_FMV *fmvs;
    };

#if TR_VERSION == 1
    // savegame settings
    struct {
        char *savegame_fmt_legacy;
        char *savegame_fmt_bson;
    };

    // global settings
    struct {
        float demo_delay;
        bool has_demo;
        char *main_menu_background_path;
        bool enable_tr2_item_drops;
        bool convert_dropped_guns;
        bool enable_killer_pushblocks;
    };

    GAME_FLOW_LEVEL_SETTINGS settings;
#elif TR_VERSION == 2
    // flow commands
    struct {
        GAME_FLOW_COMMAND cmd_init;
        GAME_FLOW_COMMAND cmd_title;
        GAME_FLOW_COMMAND cmd_death_demo_mode;
        GAME_FLOW_COMMAND cmd_death_in_game;
        GAME_FLOW_COMMAND cmd_demo_interrupt;
        GAME_FLOW_COMMAND cmd_demo_end;
    };

    // global settings
    struct {
        float demo_delay;
        bool is_demo_version;
        bool play_any_level;
        bool gym_enabled;
        bool lockout_option_ring;
        bool cheat_keys;
        bool load_save_disabled;
        int32_t single_level;
    };

    // music
    struct {
        MUSIC_TRACK_ID secret_track;
        MUSIC_TRACK_ID level_complete_track;
        MUSIC_TRACK_ID game_complete_track;
    };
#endif

    // other data
    INJECTION_DATA injections;
} GAME_FLOW;
