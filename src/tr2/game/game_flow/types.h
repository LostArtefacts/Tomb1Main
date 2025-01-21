#pragma once

#include "game/music.h"
#include "global/types.h"

// ----------------------------------------------------------------------------
// Sequencer structures
// ----------------------------------------------------------------------------

typedef enum {
    GFS_PICTURE,
    GFS_PLAY_FMV,
    GFS_PLAY_LEVEL,
    GFS_PLAY_CUTSCENE,
    GFS_LEVEL_COMPLETE,
    GFS_GAME_COMPLETE,
    GFS_PLAY_DEMO,
    GFS_SET_MUSIC_TRACK,
    GFS_SET_CAMERA_ANGLE,
    GFS_SET_START_ANIM,
    GFS_DISABLE_FLOOR,
    GFS_ENABLE_SUNSET,
    GFS_ENABLE_DEADLY_WATER,
    GFS_REMOVE_WEAPONS,
    GFS_REMOVE_AMMO,
    GFS_SET_NUM_SECRETS,
    GFS_ADD_ITEM,
    GFS_ADD_SECRET_REWARD,
} GAME_FLOW_SEQUENCE_EVENT_TYPE;

typedef struct {
    GAME_FLOW_SEQUENCE_EVENT_TYPE type;
    void *data;
} GAME_FLOW_SEQUENCE_EVENT;

typedef struct {
    int32_t length;
    GAME_FLOW_SEQUENCE_EVENT *events;
} GAME_FLOW_SEQUENCE;

// Concrete events data

typedef enum {
    GF_ADD_ITEM_PISTOLS = 0,
    GF_ADD_ITEM_SHOTGUN = 1,
    GF_ADD_ITEM_MAGNUMS = 2,
    GF_ADD_ITEM_UZIS = 3,
    GF_ADD_ITEM_HARPOON = 4,
    GF_ADD_ITEM_M16 = 5,
    GF_ADD_ITEM_GRENADE = 6,
    GF_ADD_ITEM_PISTOL_AMMO = 7,
    GF_ADD_ITEM_SHOTGUN_AMMO = 8,
    GF_ADD_ITEM_MAGNUM_AMMO = 9,
    GF_ADD_ITEM_UZI_AMMO = 10,
    GF_ADD_ITEM_HARPOON_AMMO = 11,
    GF_ADD_ITEM_M16_AMMO = 12,
    GF_ADD_ITEM_GRENADE_AMMO = 13,
    GF_ADD_ITEM_FLARES = 14,
    GF_ADD_ITEM_SMALL_MEDI = 15,
    GF_ADD_ITEM_LARGE_MEDI = 16,
    GF_ADD_ITEM_PICKUP_1 = 17,
    GF_ADD_ITEM_PICKUP_2 = 18,
    GF_ADD_ITEM_PUZZLE_1 = 19,
    GF_ADD_ITEM_PUZZLE_2 = 20,
    GF_ADD_ITEM_PUZZLE_3 = 21,
    GF_ADD_ITEM_PUZZLE_4 = 22,
    GF_ADD_ITEM_KEY_1 = 23,
    GF_ADD_ITEM_KEY_2 = 24,
    GF_ADD_ITEM_KEY_3 = 25,
    GF_ADD_ITEM_KEY_4 = 26,
    GF_ADD_ITEM_NUMBER_OF = 27,
} GF_ADD_ITEM;

typedef enum {
    GF_INV_REGULAR,
    GF_INV_SECRET,
} GF_INV_TYPE;

typedef struct {
    GF_ADD_ITEM item;
    GF_INV_TYPE inv_type;
} GFS_ADD_ITEM_DATA;

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
    const char *path;
} GAME_FLOW_CUTSCENE;

typedef struct {
    bool demo;
    const char *path;
    char *title;
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
        GAME_FLOW_CUTSCENE *cutscenes;
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
        MUSIC_TRACK_ID title_track;
        MUSIC_TRACK_ID secret_track;
        MUSIC_TRACK_ID level_complete_track;
    };

    // other data
    INJECTION_DATA injections;

    // denormalized data
    int32_t demo_level_count;
    int32_t *demo_levels;
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
