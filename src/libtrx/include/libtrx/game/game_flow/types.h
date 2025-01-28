#pragma once

#include "../music/ids.h"
#include "../objects/types.h"
#include "./enum.h"

#include <stdint.h>

typedef struct GF_COMMAND {
    GF_ACTION action;
    int32_t param;
} GF_COMMAND;

// ----------------------------------------------------------------------------
// Sequencer structures
// ----------------------------------------------------------------------------

typedef struct {
    GF_SEQUENCE_EVENT_TYPE type;
    void *data;
} GF_SEQUENCE_EVENT;

typedef struct {
    int32_t length;
    GF_SEQUENCE_EVENT *events;
} GF_SEQUENCE;

// ----------------------------------------------------------------------------
// Game flow level structures
// ----------------------------------------------------------------------------

typedef struct {
    int32_t count;
    char **data_paths;
} INJECTION_DATA;

typedef struct {
    const char *path;
} GF_FMV;

#if TR_VERSION == 1
typedef struct {
    RGB_F water_color;
    float draw_distance_fade;
    float draw_distance_max;
} GF_LEVEL_SETTINGS;

typedef struct {
    int32_t enemy_num;
    int32_t count;
    int16_t *object_ids;
} GF_DROP_ITEM_DATA;
#endif

typedef struct {
    int32_t num;
    GF_LEVEL_TYPE type;
#if TR_VERSION == 1
    char *path;
#elif TR_VERSION == 2
    const char *path;
#endif
    char *title;

    MUSIC_TRACK_ID music_track;
    GF_SEQUENCE sequence;
    INJECTION_DATA injections;

#if TR_VERSION == 1
    GF_LEVEL_SETTINGS settings;

    struct {
        uint32_t pickups;
        uint32_t kills;
        uint32_t secrets;
    } unobtainable;

    struct {
        int count;
        GF_DROP_ITEM_DATA *data;
    } item_drops;

    GAME_OBJECT_ID lara_type;
#endif
} GF_LEVEL;

typedef struct {
    int32_t count;
    GF_LEVEL *levels;
} GF_LEVEL_TABLE;

// ----------------------------------------------------------------------------
// Game flow structures
// ----------------------------------------------------------------------------

typedef struct {
    GF_LEVEL *title_level;
    GF_LEVEL_TABLE level_tables[GFLT_NUMBER_OF];

    // FMVs
    struct {
        int32_t fmv_count;
        GF_FMV *fmvs;
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
        char *main_menu_background_path;
        bool enable_tr2_item_drops;
        bool convert_dropped_guns;
        bool enable_killer_pushblocks;
    };

    GF_LEVEL_SETTINGS settings;
#elif TR_VERSION == 2
    // flow commands
    struct {
        GF_COMMAND cmd_init;
        GF_COMMAND cmd_title;
        GF_COMMAND cmd_death_demo_mode;
        GF_COMMAND cmd_death_in_game;
        GF_COMMAND cmd_demo_interrupt;
        GF_COMMAND cmd_demo_end;
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
    };
#endif

    // other data
    INJECTION_DATA injections;
} GAME_FLOW;
