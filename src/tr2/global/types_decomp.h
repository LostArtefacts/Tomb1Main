// This file is autogenerated. To update it, run tools/generate_funcs.

#pragma once

#include "const.h"

#include <libtrx/game/items.h>
#include <libtrx/game/math.h>
#include <libtrx/game/rooms/types.h>
#include <libtrx/game/text.h>

#include <stdbool.h>
#include <stdint.h>

#pragma pack(push, 1)

// clang-format off
typedef enum {
    RM_UNKNOWN  = 0,
    RM_SOFTWARE = 1,
    RM_HARDWARE = 2,
} RENDER_MODE;

typedef enum {
    AM_4_3  = 0,
    AM_16_9 = 1,
    AM_ANY  = 2,
} ASPECT_MODE;

typedef enum {
    TAM_DISABLED      = 0,
    TAM_BILINEAR_ONLY = 1,
    TAM_ALWAYS        = 2,
} TEXEL_ADJUST_MODE;

typedef struct __unaligned {
    union {
        uint8_t red;
        uint8_t r;
    };
    union {
        uint8_t green;
        uint8_t g;
    };
    union {
        uint8_t blue;
        uint8_t b;
    };
} RGB_888;

typedef struct __unaligned {
    union {
        uint8_t red;
        uint8_t r;
    };
    union {
        uint8_t green;
        uint8_t g;
    };
    union {
        uint8_t blue;
        uint8_t b;
    };
    union {
        uint8_t alpha;
        uint8_t a;
    };
} RGBA_8888;

typedef struct __unaligned {
    uint8_t index[256];
} DEPTHQ_ENTRY;

typedef struct __unaligned {
    uint8_t index[32];
} GOURAUD_ENTRY;

typedef struct __unaligned {
    XYZ_32 pos;
    XYZ_16 rot;
} PHD_3DPOS;

typedef struct __unaligned {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t r;
} SPHERE;

typedef struct __unaligned {
    float xv;
    float yv;
    float zv;
    float rhw;
    float xs;
    float ys;
    int16_t clip;
    int16_t g;
    int16_t u;
    int16_t v;
} PHD_VBUF;

typedef struct __unaligned {
    uint16_t u;
    uint16_t v;
} PHD_UV;

typedef struct __unaligned {
    uint16_t tex_page;
    uint16_t offset;
    uint16_t width;
    uint16_t height;
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
} PHD_SPRITE;

typedef enum {
    SHAPE_SPRITE = 1,
    SHAPE_LINE   = 2,
    SHAPE_BOX    = 3,
    SHAPE_FBOX   = 4,
} SHAPE;

typedef enum {
    SPRF_RGB       = 0x00FFFFFF,
    SPRF_ABS       = 0x01000000,
    SPRF_SEMITRANS = 0x02000000,
    SPRF_SCALE     = 0x04000000,
    SPRF_SHADE     = 0x08000000,
    SPRF_TINT      = 0x10000000,
    SPRF_BLEND_ADD = 0x20000000,
    SPRF_BLEND_SUB = 0x40000000,
    SPRF_BLEND_QRT = SPRF_BLEND_ADD | SPRF_BLEND_SUB,
    SPRF_BLEND     = SPRF_BLEND_QRT,
    SPRF_ITEM      = 0x80000000,
} SPRITE_FLAG;

typedef struct __unaligned {
    float xv;
    float yv;
    float zv;
    float rhw;
    float xs;
    float ys;
    float u;
    float v;
    float g;
} POINT_INFO;

typedef struct __unaligned {
    uint16_t no_selector : 1;
    uint16_t ready : 1; // not present in the OG
    uint16_t pad : 14;
    uint16_t items_count;
    uint16_t selected;
    uint16_t visible_count;
    uint16_t line_offset;
    uint16_t line_old_offset;
    uint16_t pix_width;
    uint16_t line_height;
    int16_t x_pos;
    int16_t y_pos;
    int16_t z_pos;
    uint16_t item_string_len;
    char *pitem_strings1;
    char *pitem_strings2;
    uint32_t *pitem_flags1;
    uint32_t *pitem_flags2;
    uint32_t heading_flags1;
    uint32_t heading_flags2;
    uint32_t background_flags;
    uint32_t moreup_flags;
    uint32_t moredown_flags;
    uint32_t item_flags1[24]; // MAX_REQUESTER_ITEMS
    uint32_t item_flags2[24]; // MAX_REQUESTER_ITEMS
    TEXTSTRING *heading_text1;
    TEXTSTRING *heading_text2;
    TEXTSTRING *background_text;
    TEXTSTRING *moreup_text;
    TEXTSTRING *moredown_text;
    TEXTSTRING *item_texts1[24]; // MAX_REQUESTER_ITEMS
    TEXTSTRING *item_texts2[24]; // MAX_REQUESTER_ITEMS
    char heading_string1[32];
    char heading_string2[32];
    uint32_t render_width;
    uint32_t render_height;
} REQUEST_INFO;

typedef struct __unaligned {
    uint32_t best_time[10];
    uint32_t best_finish[10];
    uint32_t finish_count;
} ASSAULT_STATS;

typedef struct __unaligned {
    int32_t _0;
    int32_t _1;
} SORT_ITEM;

typedef enum {
    DRAW_OPAQUE    = 0,
    DRAW_COLOR_KEY = 1,
} DRAW_TYPE;

typedef struct __unaligned {
    int16_t mesh_idx;
    uint16_t flags;
    BOUNDS_16 draw_bounds;
    BOUNDS_16 collision_bounds;
} STATIC_INFO;

typedef struct STATS_COMMON {
    uint32_t timer;
    uint32_t ammo_used;
    uint32_t ammo_hits;
    uint32_t distance;
    uint16_t kills;
    uint8_t medipacks;
} STATS_COMMON;

typedef struct {
    struct STATS_COMMON;
    int32_t found_secrets;
    int32_t total_secrets;
} FINAL_STATS;

typedef struct {
    struct STATS_COMMON;
    uint8_t secrets_bitmap;
} LEVEL_STATS;

typedef struct __unaligned {
    uint16_t pistol_ammo;
    uint16_t magnum_ammo;
    uint16_t uzi_ammo;
    uint16_t shotgun_ammo;
    uint16_t m16_ammo;
    uint16_t grenade_ammo;
    uint16_t harpoon_ammo;
    uint8_t small_medipacks;
    uint8_t large_medipacks;
    uint8_t reserved1;
    uint8_t flares;
    uint8_t gun_status;
    uint8_t gun_type;
    uint16_t available:   1; // 0x01 1
    uint16_t has_pistols: 1; // 0x02 2
    uint16_t has_magnums: 1; // 0x04 4
    uint16_t has_uzis:    1; // 0x08 8
    uint16_t has_shotgun: 1; // 0x10 16
    uint16_t has_m16:     1; // 0x20 32
    uint16_t has_grenade: 1; // 0x40 64
    uint16_t has_harpoon: 1; // 0x80 128
    uint16_t pad : 8;
    uint16_t reserved2;
    LEVEL_STATS stats;
} START_INFO;

typedef struct __unaligned {
    START_INFO start[24];
    LEVEL_STATS current_stats;
    int16_t current_level;
    bool bonus_flag;
    uint8_t num_pickup[2];
    uint8_t num_puzzle[4];
    uint8_t num_key[4];
    uint16_t reserved;
    char buffer[6272]; // MAX_SG_BUFFER_SIZE
} SAVEGAME_INFO;

typedef struct __unaligned {
    int16_t lock_angles[4];
    int16_t left_angles[4];
    int16_t right_angles[4];
    int16_t aim_speed;
    int16_t shot_accuracy;
    int32_t gun_height;
    int32_t damage;
    int32_t target_dist;
    int16_t recoil_frame;
    int16_t flash_time;
    int16_t sample_num;
} WEAPON_INFO;

typedef struct __unaligned {
    int16_t zone_num;
    int16_t enemy_zone_num;
    int32_t distance;
    int32_t ahead;
    int32_t bite;
    int16_t angle;
    int16_t enemy_facing;
} AI_INFO;

typedef enum {
    PM_SPINE    = 1,
    PM_FRONT    = 2,
    PM_IN_FRONT = 4,
    PM_PAGE_2   = 8,
    PM_BACK     = 16,
    PM_IN_BACK  = 32,
    PM_PAGE_1   = 64,
    PM_COMMON   = PM_SPINE | PM_BACK | PM_FRONT,
} PASS_MESH;

typedef enum {
    GFE_PICTURE          = 0,
    GFE_LIST_START       = 1,
    GFE_LIST_END         = 2,
    GFE_PLAY_FMV         = 3,
    GFE_START_LEVEL      = 4,
    GFE_CUTSCENE         = 5,
    GFE_LEVEL_COMPLETE   = 6,
    GFE_DEMO_PLAY        = 7,
    GFE_JUMP_TO_SEQ      = 8,
    GFE_END_SEQ          = 9,
    GFE_SET_TRACK        = 10,
    GFE_SUNSET           = 11,
    GFE_LOADING_PIC      = 12,
    GFE_DEADLY_WATER     = 13,
    GFE_REMOVE_WEAPONS   = 14,
    GFE_GAME_COMPLETE    = 15,
    GFE_CUT_ANGLE        = 16,
    GFE_NO_FLOOR         = 17,
    GFE_ADD_TO_INV       = 18,
    GFE_START_ANIM       = 19,
    GFE_NUM_SECRETS      = 20,
    GFE_KILL_TO_COMPLETE = 21,
    GFE_REMOVE_AMMO      = 22,
} GF_EVENTS;

typedef enum {
    TARGET_NONE      = 0,
    TARGET_PRIMARY   = 1,
    TARGET_SECONDARY = 2,
} TARGET_TYPE;

typedef struct __unaligned {
    XYZ_32 pos;
    int32_t mesh_num;
} BITE;

typedef enum {
    RF_UNDERWATER  = 0x01,
    RF_OUTSIDE     = 0x08,
    RF_DYNAMIC_LIT = 0x10,
    RF_NOT_INSIDE  = 0x20,
    RF_INSIDE      = 0x40,
} ROOM_FLAG;

typedef struct __unaligned {
    SECTOR *sector;
    SECTOR old_sector;
    int16_t block;
} DOORPOS_DATA;

typedef enum {
    CAM_CHASE     = 0,
    CAM_FIXED     = 1,
    CAM_LOOK      = 2,
    CAM_COMBAT    = 3,
    CAM_CINEMATIC = 4,
    CAM_HEAVY     = 5,
} CAMERA_TYPE;

typedef struct __unaligned {
    union {
        XYZ_32 pos;
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
    };
    int16_t room_num;
    int16_t box_num;
} GAME_VECTOR;

typedef struct __unaligned {
    union {
        struct __unaligned {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        XYZ_32 pos;
    };
    int16_t data;
    int16_t flags;
} OBJECT_VECTOR;

typedef struct __unaligned {
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
    int16_t height;
    int16_t overlap_index;
} BOX_INFO;

typedef enum {
    LV_GYM   = 0,
    LV_FIRST = 1,
} LEVEL_TYPE;

typedef enum {
    COLOR_BLACK      = 0,
    COLOR_GRAY       = 1,
    COLOR_WHITE      = 2,
    COLOR_RED        = 3,
    COLOR_ORANGE     = 4,
    COLOR_YELLOW     = 5,
    COLOR_DARK_GREEN = 12,
    COLOR_GREEN      = 13,
    COLOR_CYAN       = 14,
    COLOR_BLUE       = 15,
    COLOR_MAGENTA    = 16,
    COLOR_NUMBER_OF  = 17,
} COLOR_NAME;

typedef struct {
    RGB_888 rgb;
    uint8_t palette_index;
} NAMED_COLOR;

typedef enum {
    INV_GAME_MODE  = 0,
    INV_TITLE_MODE = 1,
    INV_KEYS_MODE  = 2,
    INV_SAVE_MODE  = 3,
    INV_LOAD_MODE  = 4,
    INV_DEATH_MODE = 5,
} INVENTORY_MODE;

typedef enum {
    GAMEMODE_NOT_IN_GAME,
    GAMEMODE_IN_GAME,
    GAMEMODE_IN_DEMO,
    GAMEMODE_IN_CUTSCENE
} GAMEMODE;

typedef enum {
    TRAP_SET      = 0,
    TRAP_ACTIVATE = 1,
    TRAP_WORKING  = 2,
    TRAP_FINISHED = 3,
} TRAP_ANIM;

typedef enum {
    GFD_START_GAME       = 0x0000,
    GFD_START_SAVED_GAME = 0x0100,
    GFD_START_CINE       = 0x0200,
    GFD_START_FMV        = 0x0300,
    GFD_START_DEMO       = 0x0400,
    GFD_EXIT_TO_TITLE    = 0x0500,
    GFD_LEVEL_COMPLETE   = 0x0600,
    GFD_EXIT_GAME        = 0x0700,
    GFD_EXIT_TO_OPTION   = 0x0800,
    GFD_TITLE_DESELECT   = 0x0900,
    GFD_OVERRIDE         = 0x0A00,
} GAME_FLOW_DIR;

typedef struct __unaligned {
    int32_t first_option;
    int32_t title_replace;
    int32_t on_death_demo_mode;
    int32_t on_death_in_game;
    int32_t no_input_time;
    int32_t on_demo_interrupt;
    int32_t on_demo_end;
    uint16_t reserved1[18];
    uint16_t num_levels;
    uint16_t num_pictures;
    uint16_t num_titles;
    uint16_t num_fmvs;
    uint16_t num_cutscenes;
    uint16_t num_demos;
    uint16_t title_track;
    int16_t single_level;
    uint16_t reserved2[16];

    uint16_t demo_version:              1;
    uint16_t title_disabled:            1;
    uint16_t cheat_mode_check_disabled: 1;
    uint16_t load_save_disabled:        1;
    uint16_t screen_sizing_disabled:    1;
    uint16_t lockout_option_ring:       1;
    uint16_t dozy_cheat_enabled:        1;
    uint16_t cyphered_strings:          1;
    uint16_t gym_enabled:               1;
    uint16_t play_any_level:            1;
    uint16_t cheat_enable:              1;

    uint16_t reserved3[3];
    uint8_t cypher_code;
    uint8_t language;
    uint8_t secret_track;
    uint8_t level_complete_track;
    uint16_t reserved4[2];
} GAME_FLOW;

typedef struct __unaligned {
    GAME_VECTOR pos;
    GAME_VECTOR target;
    CAMERA_TYPE type;
    int32_t shift;
    uint32_t flags;
    int32_t fixed_camera;
    int32_t num_frames;
    int32_t bounce;
    int32_t underwater;
    int32_t target_distance;
    int32_t target_square;
    int16_t target_angle;
    int16_t actual_angle;
    int16_t target_elevation;
    int16_t box;
    int16_t num;
    int16_t last;
    int16_t timer;
    int16_t speed;
    ITEM *item;
    ITEM *last_item;
    OBJECT_VECTOR *fixed;
    int32_t is_lara_mic; // TODO: remove this - now stored in g_Config
    XYZ_32 mic_pos;
} CAMERA_INFO;

typedef enum {
    CF_NORMAL        = 0,
    CF_FOLLOW_CENTRE = 1,
    CF_NO_CHUNKY     = 2,
    CF_CHASE_OBJECT  = 3,
} CAMERA_FLAGS;

typedef enum {
    FBBOX_MIN_X = 0,
    FBBOX_MAX_X = 1,
    FBBOX_MIN_Y = 2,
    FBBOX_MAX_Y = 3,
    FBBOX_MIN_Z = 4,
    FBBOX_MAX_Z = 5,
    FBBOX_X     = 6,
    FBBOX_Y     = 7,
    FBBOX_Z     = 8,
    FBBOX_ROT   = 9,
} FRAME_BBOX_INFO;

typedef struct __unaligned {
    union {
        int32_t flags;
        struct {
            uint32_t matrix_pop:  1;
            uint32_t matrix_push: 1;
            uint32_t rot_x:       1;
            uint32_t rot_y:       1;
            uint32_t rot_z:       1;
            uint32_t pad:         11;
        };
    };
    XYZ_32 pos;
} BONE;

typedef enum {
    BF_MATRIX_POP  = 1,
    BF_MATRIX_PUSH = 2,
    BF_ROT_X       = 4,
    BF_ROT_Y       = 8,
    BF_ROT_Z       = 16,
} BONE_FLAGS;

typedef struct __unaligned {
    int16_t tx;
    int16_t ty;
    int16_t tz;
    int16_t cx;
    int16_t cy;
    int16_t cz;
    int16_t fov;
    int16_t roll;
} CINE_FRAME;

typedef struct __unaligned {
    uint16_t key[14]; // INPUT_ROLE_NUMBER_OF
} CONTROL_LAYOUT;

typedef enum {
    COLL_NONE      = 0x00,
    COLL_FRONT     = 0x01,
    COLL_LEFT      = 0x02,
    COLL_RIGHT     = 0x04,
    COLL_TOP       = 0x08,
    COLL_TOP_FRONT = 0x10,
    COLL_CLAMP     = 0x20,
} COLL_TYPE;

typedef enum {
    HT_WALL        = 0,
    HT_SMALL_SLOPE = 1,
    HT_BIG_SLOPE   = 2,
} HEIGHT_TYPE;

typedef struct __unaligned {
    int16_t number;
    int16_t volume;
    int16_t randomness;
    int16_t flags;
} SAMPLE_INFO;

/*
typedef struct __unaligned {
    int32_t volume;
    int32_t pan;
    int32_t sample_num;
    int32_t pitch;
} SOUND_SLOT;
*/

typedef enum {
    SF_FLIP = 0x40,
    SF_UNFLIP = 0x80,
} SOUND_FLAG;

typedef enum {
    AC_NULL          = 0,
    AC_MOVE_ORIGIN   = 1,
    AC_JUMP_VELOCITY = 2,
    AC_ATTACK_READY  = 3,
    AC_DEACTIVATE    = 4,
    AC_SOUND_FX      = 5,
    AC_EFFECT        = 6,
} ANIM_COMMAND;

typedef enum {
    ACE_ALL   = 0,
    ACE_LAND  = 1,
    ACE_WATER = 2,
} ANIM_COMMAND_ENVIRONMENT;

typedef struct __unaligned {
    int32_t boat_turn;
    int32_t left_fallspeed;
    int32_t right_fallspeed;
    int16_t tilt_angle;
    int16_t extra_rotation;
    int32_t water;
    int32_t pitch;
} BOAT_INFO;

typedef struct __unaligned {
    int16_t track_mesh;
    int32_t skidoo_turn;
    int32_t left_fallspeed;
    int32_t right_fallspeed;
    int16_t momentum_angle;
    int16_t extra_rotation;
    int32_t pitch;
} SKIDOO_INFO;

typedef struct __unaligned {
    struct {
        XYZ_16 min;
        XYZ_16 max;
    } shift, rot;
} OBJECT_BOUNDS;

typedef struct __unaligned {
    int32_t xv;
    int32_t yv;
    int32_t zv;
} PORTAL_VBUF;

typedef struct __unaligned {
    BOUNDS_16 bounds;
    XYZ_16 offset;
    int16_t mesh_rots[];
} FRAME_INFO;

typedef struct __unaligned {
    int32_t table[32]; // WIBBLE_SIZE
} ROOM_LIGHT_TABLE;

typedef enum {
    GF_S_PC_DETAIL_LEVELS      = 0,
    GF_S_PC_DEMO_MODE          = 1,
    GF_S_PC_SOUND              = 2,
    GF_S_PC_CONTROLS           = 3,
    GF_S_PC_GAMMA              = 4,
    GF_S_PC_SET_VOLUMES        = 5,
    GF_S_PC_USER_KEYS          = 6,
    GF_S_PC_SAVE_FILE_WARNING  = 7,
    GF_S_PC_TRY_AGAIN_QUESTION = 8,
    GF_S_PC_YES                = 9,
    GF_S_PC_NO                 = 10,
    GF_S_PC_SAVE_COMPLETE      = 11,
    GF_S_PC_NO_SAVE_GAMES      = 12,
    GF_S_PC_NONE_VALID         = 13,
    GF_S_PC_SAVE_GAME_QUESTION = 14,
    GF_S_PC_EMPTY_SLOT         = 15,
    GF_S_PC_OFF                = 16,
    GF_S_PC_ON                 = 17,
    GF_S_PC_SETUP_SOUND_CARD   = 18,
    GF_S_PC_DEFAULT_KEYS       = 19,
    GF_S_PC_DOZY               = 20,
    GF_S_PC_NUMBER_OF          = 41,
} GF_PC_STRING;

typedef enum {
    GF_S_GAME_HEADING_INVENTORY       = 0,
    GF_S_GAME_HEADING_OPTION          = 1,
    GF_S_GAME_HEADING_ITEMS           = 2,
    GF_S_GAME_HEADING_GAME_OVER       = 3,
    GF_S_GAME_PASSPORT_LOAD_GAME      = 4,
    GF_S_GAME_PASSPORT_SAVE_GAME      = 5,
    GF_S_GAME_PASSPORT_NEW_GAME       = 6,
    GF_S_GAME_PASSPORT_RESTART_LEVEL  = 7,
    GF_S_GAME_PASSPORT_EXIT_TO_TITLE  = 8,
    GF_S_GAME_PASSPORT_EXIT_DEMO      = 9,
    GF_S_GAME_PASSPORT_EXIT_GAME      = 10,
    GF_S_GAME_PASSPORT_SELECT_LEVEL   = 11,
    GF_S_GAME_PASSPORT_SAVE_POSITION  = 12,
    GF_S_GAME_DETAIL_SELECT_DETAIL    = 13,
    GF_S_GAME_DETAIL_HIGH             = 14,
    GF_S_GAME_DETAIL_MEDIUM           = 15,
    GF_S_GAME_DETAIL_LOW              = 16,
    GF_S_GAME_KEYMAP_WALK             = 17,
    GF_S_GAME_KEYMAP_ROLL             = 18,
    GF_S_GAME_KEYMAP_RUN              = 19,
    GF_S_GAME_KEYMAP_LEFT             = 20,
    GF_S_GAME_KEYMAP_RIGHT            = 21,
    GF_S_GAME_KEYMAP_BACK             = 22,
    GF_S_GAME_KEYMAP_STEP_LEFT        = 23,
    GF_S_GAME_KEYMAP_RESERVED_1       = 24,
    GF_S_GAME_KEYMAP_STEP_RIGHT       = 25,
    GF_S_GAME_KEYMAP_RESERVED_2       = 26,
    GF_S_GAME_KEYMAP_LOOK             = 27,
    GF_S_GAME_KEYMAP_JUMP             = 28,
    GF_S_GAME_KEYMAP_ACTION           = 29,
    GF_S_GAME_KEYMAP_DRAW_WEAPON      = 30,
    GF_S_GAME_KEYMAP_RESERVED_3       = 31,
    GF_S_GAME_KEYMAP_INVENTORY        = 32,
    GF_S_GAME_KEYMAP_FLARE            = 33,
    GF_S_GAME_KEYMAP_STEP             = 34,
    GF_S_GAME_INV_ITEM_STATISTICS     = 35,
    GF_S_GAME_INV_ITEM_PISTOLS        = 36,
    GF_S_GAME_INV_ITEM_SHOTGUN        = 37,
    GF_S_GAME_INV_ITEM_MAGNUMS        = 38,
    GF_S_GAME_INV_ITEM_UZIS           = 39,
    GF_S_GAME_INV_ITEM_HARPOON        = 40,
    GF_S_GAME_INV_ITEM_M16            = 41,
    GF_S_GAME_INV_ITEM_GRENADE        = 42,
    GF_S_GAME_INV_ITEM_FLARE          = 43,
    GF_S_GAME_INV_ITEM_PISTOL_AMMO    = 44,
    GF_S_GAME_INV_ITEM_SHOTGUN_AMMO   = 45,
    GF_S_GAME_INV_ITEM_MAGNUM_AMMO    = 46,
    GF_S_GAME_INV_ITEM_UZI_AMMO       = 47,
    GF_S_GAME_INV_ITEM_HARPOON_AMMO   = 48,
    GF_S_GAME_INV_ITEM_M16_AMMO       = 49,
    GF_S_GAME_INV_ITEM_GRENADE_AMMO   = 50,
    GF_S_GAME_INV_ITEM_SMALL_MEDIPACK = 51,
    GF_S_GAME_INV_ITEM_LARGE_MEDIPACK = 52,
    GF_S_GAME_INV_ITEM_PICKUP         = 53,
    GF_S_GAME_INV_ITEM_PUZZLE         = 54,
    GF_S_GAME_INV_ITEM_KEY            = 55,
    GF_S_GAME_INV_ITEM_GAME           = 56,
    GF_S_GAME_INV_ITEM_LARA_HOME      = 57,
    GF_S_GAME_MISC_LOADING            = 58,
    GF_S_GAME_MISC_TIME_TAKEN         = 59,
    GF_S_GAME_MISC_SECRETS_FOUND      = 60,
    GF_S_GAME_MISC_LOCATION           = 61,
    GF_S_GAME_MISC_KILLS              = 62,
    GF_S_GAME_MISC_AMMO_USED          = 63,
    GF_S_GAME_MISC_HITS               = 64,
    GF_S_GAME_MISC_SAVES_PERFORMED    = 65,
    GF_S_GAME_MISC_DISTANCE_TRAVELLED = 66,
    GF_S_GAME_MISC_HEALTH_PACKS_USED  = 67,
    GF_S_GAME_MISC_RELEASE_VERSION    = 68,
    GF_S_GAME_MISC_NONE               = 69,
    GF_S_GAME_MISC_FINISH             = 70,
    GF_S_GAME_MISC_BEST_TIMES         = 71,
    GF_S_GAME_MISC_NO_TIMES_SET       = 72,
    GF_S_GAME_MISC_NA                 = 73,
    GF_S_GAME_MISC_CURRENT_POSITION   = 74,
    GF_S_GAME_MISC_FINAL_STATISTICS   = 75,
    GF_S_GAME_MISC_OF                 = 76,
    GF_S_GAME_MISC_STORY_SO_FAR       = 77,
    GF_S_GAME_NUMBER_OF               = 89,
} GF_GAME_STRING;

typedef enum {
    GF_ADD_INV_PISTOLS      = 0,
    GF_ADD_INV_SHOTGUN      = 1,
    GF_ADD_INV_MAGNUMS      = 2,
    GF_ADD_INV_UZIS         = 3,
    GF_ADD_INV_HARPOON      = 4,
    GF_ADD_INV_M16          = 5,
    GF_ADD_INV_GRENADE      = 6,
    GF_ADD_INV_PISTOL_AMMO  = 7,
    GF_ADD_INV_SHOTGUN_AMMO = 8,
    GF_ADD_INV_MAGNUM_AMMO  = 9,
    GF_ADD_INV_UZI_AMMO     = 10,
    GF_ADD_INV_HARPOON_AMMO = 11,
    GF_ADD_INV_M16_AMMO     = 12,
    GF_ADD_INV_GRENADE_AMMO = 13,
    GF_ADD_INV_FLARES       = 14,
    GF_ADD_INV_SMALL_MEDI   = 15,
    GF_ADD_INV_LARGE_MEDI   = 16,
    GF_ADD_INV_PICKUP_1     = 17,
    GF_ADD_INV_PICKUP_2     = 18,
    GF_ADD_INV_PUZZLE_1     = 19,
    GF_ADD_INV_PUZZLE_2     = 20,
    GF_ADD_INV_PUZZLE_3     = 21,
    GF_ADD_INV_PUZZLE_4     = 22,
    GF_ADD_INV_KEY_1        = 23,
    GF_ADD_INV_KEY_2        = 24,
    GF_ADD_INV_KEY_3        = 25,
    GF_ADD_INV_KEY_4        = 26,
    GF_ADD_INV_NUMBER_OF    = 27,
} GF_ADD_INV;

typedef enum {
    IT_NAME      = 0,
    IT_QTY       = 1,
    IT_NUMBER_OF = 2,
} INV_TEXT;

typedef enum {
    REQ_CENTER      = 0x00,
    REQ_USE         = 0x01,
    REQ_ALIGN_LEFT  = 0x02,
    REQ_ALIGN_RIGHT = 0x04,
    REQ_HEADING     = 0x08,
    REQ_BEST_TIME   = 0x10,
    REQ_NORMAL_TIME = 0x20,
    REQ_NO_TIME     = 0x40,
} REQUESTER_FLAGS;

// clang-format on

#pragma pack(pop)
