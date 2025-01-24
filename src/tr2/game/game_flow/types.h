#pragma once

#include "./enum.h"
#include "game/music.h"
#include "global/types.h"

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
    float duration;
} GAME_FLOW_DISPLAY_PICTURE_DATA;

typedef enum {
    GF_INV_REGULAR,
    GF_INV_SECRET,
} GAME_FLOW_INV_TYPE;

typedef struct {
    GAME_OBJECT_ID object_id;
    GAME_FLOW_INV_TYPE inv_type;
    int32_t qty;
} GAME_FLOW_ADD_ITEM_DATA;

// ----------------------------------------------------------------------------
// Game flow structures
// ----------------------------------------------------------------------------

typedef struct {
    int32_t count;
    char **data_paths;
} INJECTION_DATA;

typedef struct {
    const char *path;
} GAME_FLOW_FMV;

typedef struct {
    int32_t num;
    GAME_FLOW_LEVEL_TYPE type;
    const char *path;
    char *title;

    MUSIC_TRACK_ID music_track;
    GAME_FLOW_SEQUENCE sequence;
    INJECTION_DATA injections;
} GAME_FLOW_LEVEL;

typedef struct {
    // levels
    GAME_FLOW_LEVEL *title_level;

    struct {
        int32_t level_count;
        GAME_FLOW_LEVEL *levels;
    };

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

    // FMVs
    struct {
        int32_t fmv_count;
        GAME_FLOW_FMV *fmvs;
    };

    // flow commands
    struct {
        GAME_FLOW_COMMAND cmd_init;
        GAME_FLOW_COMMAND cmd_title;
        GAME_FLOW_COMMAND cmd_death_demo_mode;
        GAME_FLOW_COMMAND cmd_death_in_game;
        GAME_FLOW_COMMAND cmd_demo_interrupt;
        GAME_FLOW_COMMAND cmd_demo_end;
    };

    // global flags
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

    // other data
    INJECTION_DATA injections;
} GAME_FLOW;

// ----------------------------------------------------------------------------
// Game information
// ----------------------------------------------------------------------------

typedef struct {
    struct {
        GAME_FLOW_LEVEL_TYPE type;
        int32_t num;
    } current_level;
} GAME_INFO;
