#pragma once

#include "game/music.h"
#include "global/types.h"

typedef struct {
    int32_t count;
    char **data_paths;
} INJECTION_DATA;

typedef struct {
    struct {
        GAME_FLOW_LEVEL_TYPE type;
        int32_t num;
    } current_level;
} GAME_INFO;

typedef struct {
    const char *path;
} GAME_FLOW_FMV;

typedef struct {
    bool demo;
    const char *path;
    char *title;

    INJECTION_DATA injections;
} GAME_FLOW_LEVEL;

typedef struct {
    int32_t level_count;
    GAME_FLOW_LEVEL *levels;

    int32_t fmv_count;
    GAME_FLOW_FMV *fmvs;

    INJECTION_DATA injections;

    GAME_FLOW_COMMAND first_option;
    GAME_FLOW_COMMAND title_replace;
    GAME_FLOW_COMMAND on_death_demo_mode;
    GAME_FLOW_COMMAND on_death_in_game;
    GAME_FLOW_COMMAND on_demo_interrupt;
    GAME_FLOW_COMMAND on_demo_end;

    float demo_delay;
    bool is_demo_version;
    bool title_disabled;
    bool play_any_level;
    bool gym_enabled;
    bool lockout_option_ring;
    bool cheat_keys;
    bool load_save_disabled;
    int32_t single_level;

    MUSIC_TRACK_ID title_track;
    MUSIC_TRACK_ID secret_track;
    MUSIC_TRACK_ID level_complete_track;

    // denormalized data
    int32_t demo_level_count;
    int32_t *demo_levels;
} GAME_FLOW;
