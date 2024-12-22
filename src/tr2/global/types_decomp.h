// This file is autogenerated. To update it, run tools/generate_funcs.

#pragma once

#include "const.h"

#include <libtrx/game/items.h>
#include <libtrx/game/lara/types.h>
#include <libtrx/game/math.h>
#include <libtrx/game/rooms/types.h>
#include <libtrx/game/text.h>

#include <stdbool.h>
#include <stdint.h>

#pragma pack(push, 1)

// clang-format off
typedef struct __unaligned {
    int32_t _00;
    int32_t _01;
    int32_t _02;
    int32_t _03;
    int32_t _10;
    int32_t _11;
    int32_t _12;
    int32_t _13;
    int32_t _20;
    int32_t _21;
    int32_t _22;
    int32_t _23;
} MATRIX;

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

typedef enum {
    POLY_GTMAP        = 0,
    POLY_WGTMAP       = 1,
    POLY_GTMAP_PERSP  = 2,
    POLY_WGTMAP_PERSP = 3,
    POLY_LINE         = 4,
    POLY_FLAT         = 5,
    POLY_GOURAUD      = 6,
    POLY_TRANS        = 7,
    POLY_SPRITE       = 8,
    POLY_HWR_GTMAP    = 9,
    POLY_HWR_WGTMAP   = 10,
    POLY_HWR_GOURAUD  = 11,
    POLY_HWR_LINE     = 12,
    POLY_HWR_TRANS    = 13,
} POLY_TYPE;

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

typedef struct __unaligned {
    uint32_t timer;
    uint32_t shots;
    uint32_t hits;
    uint32_t distance;
    uint16_t kills;
    uint8_t secrets;
    uint8_t medipacks;
} STATISTICS_INFO;

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
    STATISTICS_INFO statistics;
} START_INFO;

typedef struct __unaligned {
    START_INFO start[24];
    STATISTICS_INFO statistics;
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

typedef struct __unaligned {
    int16_t timer;
    int16_t sprite;
} PICKUP_INFO;

typedef struct __unaligned {
    int16_t shape;
    XYZ_16 pos;
    int32_t param1;
    int32_t param2;
    void *grdptr;
    int16_t sprite_num;
} INVENTORY_SPRITE;

typedef struct __unaligned {
    char *string;
    int16_t object_id;
    int16_t frames_total;
    int16_t current_frame;
    int16_t goal_frame;
    int16_t open_frame;
    int16_t anim_direction;
    int16_t anim_speed;
    int16_t anim_count;
    int16_t x_rot_pt_sel;
    int16_t x_rot_pt;
    int16_t x_rot_sel;
    int16_t x_rot_nosel;
    int16_t x_rot;
    int16_t y_rot_sel;
    int16_t y_rot;
    int32_t y_trans_sel;
    int32_t y_trans;
    int32_t z_trans_sel;
    int32_t z_trans;
    uint32_t meshes_sel;
    uint32_t meshes_drawn;
    int16_t inv_pos;
    INVENTORY_SPRITE **sprite_list;
    int32_t reserved[4];
} INVENTORY_ITEM;

typedef enum {
    RNG_OPENING           = 0,
    RNG_OPEN              = 1,
    RNG_CLOSING           = 2,
    RNG_MAIN2OPTION       = 3,
    RNG_MAIN2KEYS         = 4,
    RNG_KEYS2MAIN         = 5,
    RNG_OPTION2MAIN       = 6,
    RNG_SELECTING         = 7,
    RNG_SELECTED          = 8,
    RNG_DESELECTING       = 9,
    RNG_DESELECT          = 10,
    RNG_CLOSING_ITEM      = 11,
    RNG_EXITING_INVENTORY = 12,
    RNG_DONE              = 13,
} RING_STATUS;

typedef struct __unaligned {
    int16_t count;
    int16_t status;
    int16_t status_target;
    int16_t radius_target;
    int16_t radius_rate;
    int16_t camera_y_target;
    int16_t camera_y_rate;
    int16_t camera_pitch_target;
    int16_t camera_pitch_rate;
    int16_t rotate_target;
    int16_t rotate_rate;
    int16_t item_pt_x_rot_target;
    int16_t item_pt_x_rot_rate;
    int16_t item_x_rot_target;
    int16_t item_x_rot_rate;
    int32_t item_y_trans_target;
    int32_t item_y_trans_rate;
    int32_t item_z_trans_target;
    int32_t item_z_trans_rate;
    int32_t misc;
} IMOTION_INFO;

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

typedef struct __unaligned {
    INVENTORY_ITEM **list;
    int16_t type;
    int16_t radius;
    int16_t camera_pitch;
    int16_t rotating;
    int16_t rot_count;
    int16_t current_object;
    int16_t target_object;
    int16_t number_of_objects;
    int16_t angle_adder;
    int16_t rot_adder;
    int16_t rot_adder_l;
    int16_t rot_adder_r;
    PHD_3DPOS ring_pos;
    PHD_3DPOS camera;
    XYZ_32 light;
    IMOTION_INFO *imo;
} RING_INFO;

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
    RT_MAIN   = 0,
    RT_OPTION = 1,
    RT_KEYS   = 2,
} RING_TYPE;

typedef enum {
    INV_COLOR_BLACK      = 0,
    INV_COLOR_GRAY       = 1,
    INV_COLOR_WHITE      = 2,
    INV_COLOR_RED        = 3,
    INV_COLOR_ORANGE     = 4,
    INV_COLOR_YELLOW     = 5,
    INV_COLOR_DARK_GREEN = 12,
    INV_COLOR_GREEN      = 13,
    INV_COLOR_CYAN       = 14,
    INV_COLOR_BLUE       = 15,
    INV_COLOR_MAGENTA    = 16,
    INV_COLOR_NUMBER_OF  = 17,
} INV_COLOR;

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

    uint16_t demo_version:              1; // 0x0001
    uint16_t title_disabled:            1; // 0x0002
    uint16_t cheat_mode_check_disabled: 1; // 0x0004
    uint16_t no_input_timeout:          1; // 0x0008
    uint16_t load_save_disabled:        1; // 0x0010
    uint16_t screen_sizing_disabled:    1; // 0x0020
    uint16_t lockout_option_ring:       1; // 0x0040
    uint16_t dozy_cheat_enabled:        1; // 0x0080
    uint16_t cyphered_strings:          1; // 0x0100
    uint16_t gym_enabled:               1; // 0x0200
    uint16_t play_any_level:            1; // 0x0400
    uint16_t cheat_enable:              1; // 0x0800

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
    SFX_LARA_FEET = 0,
    SFX_LARA_CLIMB_2 = 1,
    SFX_LARA_NO = 2,
    SFX_LARA_SLIPPING = 3,
    SFX_LARA_LAND = 4,
    SFX_LARA_CLIMB_1 = 5,
    SFX_LARA_DRAW = 6,
    SFX_LARA_HOLSTER = 7,
    SFX_LARA_FIRE = 8,
    SFX_LARA_RELOAD = 9,
    SFX_LARA_RICOCHET = 10,
    SFX_LARA_FLARE_IGNITE = 11,
    SFX_LARA_FLARE_BURN = 12,
    SFX_LARA_HARPOON_FIRE = 15,
    SFX_LARA_HARPOON_LOAD = 16,
    SFX_LARA_WET_FEET = 17,
    SFX_LARA_WADE = 18,
    SFX_LARA_TREAD = 20,
    SFX_LARA_FIRE_MAGNUMS = 21,
    SFX_LARA_HARPOON_LOAD_WATER = 22,
    SFX_LARA_HARPOON_FIRE_WATER = 23,
    SFX_MASSIVE_CRASH = 24,
    SFX_PUSH_SWITCH = 25,
    SFX_LARA_CLIMB_3 = 26,
    SFX_LARA_BODYSL = 27,
    SFX_LARA_SHIMMY = 28,
    SFX_LARA_JUMP = 29,
    SFX_LARA_FALL = 30,
    SFX_LARA_INJURY = 31,
    SFX_LARA_ROLL = 32,
    SFX_LARA_SPLASH = 33,
    SFX_LARA_GETOUT = 34,
    SFX_LARA_SWIM = 35,
    SFX_LARA_BREATH = 36,
    SFX_LARA_BUBBLES = 37,
    SFX_LARA_SWITCH = 38,
    SFX_LARA_KEY = 39,
    SFX_LARA_OBJECT = 40,
    SFX_LARA_GENERAL_DEATH = 41,
    SFX_LARA_KNEES_DEATH = 42,
    SFX_LARA_UZI_FIRE = 43,
    SFX_LARA_UZI_STOP = 44,
    SFX_LARA_SHOTGUN = 45,
    SFX_LARA_BLOCK_PUSH_1 = 46,
    SFX_LARA_BLOCK_PUSH_2 = 47,
    SFX_CLICK = 48,
    SFX_LARA_HIT = 49,
    SFX_LARA_BULLETHIT = 50,
    SFX_LARA_BLKPULL = 51,
    SFX_LARA_FLOATING = 52,
    SFX_LARA_FALLDETH = 53,
    SFX_LARA_GRABHAND = 54,
    SFX_LARA_GRABBODY = 55,
    SFX_LARA_GRABFEET = 56,
    SFX_LARA_SWITCHUP = 57,
    SFX_GLASS_BREAK = 58,
    SFX_WATER_LOOP = 59,
    SFX_UNDERWATER = 60,
    SFX_UNDERWATER_SWITCH = 61,
    SFX_LARA_PICKUP = 62,
    SFX_BLOCK_SOUND = 63,
    SFX_DOOR = 64,
    SFX_SWING = 65,
    SFX_ROCK_FALL_CRUMBLE = 66,
    SFX_ROCK_FALL_LAND = 67,
    SFX_ROCK_FALL_SOLID = 68,
    SFX_ENEMY_FEET = 69,
    SFX_ENEMY_GRUNT = 70,
    SFX_ENEMY_HIT_1 = 71,
    SFX_ENEMY_HIT_2 = 72,
    SFX_ENEMY_DEATH_1 = 73,
    SFX_ENEMY_JUMP = 74,
    SFX_ENEMY_CLIMBUP = 75,
    SFX_ENEMY_CLIMBDOWN = 76,
    SFX_WEAPON_CLATTER = 77,
    SFX_M16_FIRE = 78,
    SFX_WATERFALL_LOOP = 79,
    SFX_SWORD_STATUE_DROP = 80,
    SFX_SWORD_STATUE_LIFT = 81,
    SFX_PORTCULLIS_UP = 82,
    SFX_PORTCULLIS_DOWN = 83,
    SFX_DOG_FEET_1 = 84,
    SFX_BODY_SLAM = 85,
    SFX_DOG_BARK_1 = 86,
    SFX_DOG_FEET_2 = 87,
    SFX_DOG_BARK_2 = 88,
    SFX_DOG_DEATH = 89,
    SFX_DOG_PANT = 90,
    SFX_LEOPARD_FEET = 91,
    SFX_LEOPARD_ROAR = 92,
    SFX_LEOPARD_BITE = 93,
    SFX_LEOPARD_STRIKE = 94,
    SFX_LEOPARD_DEATH = 95,
    SFX_LEOPARD_GROWL = 96,
    SFX_RAT_ATTACK = 97,
    SFX_RAT_DEATH = 98,
    SFX_TIGER_ROAR = 99,
    SFX_TIGER_BITE = 100,
    SFX_TIGER_STRIKE = 101,
    SFX_TIGER_DEATH = 102,
    SFX_TIGER_GROWL = 103,
    SFX_M16_STOP = 104,
    SFX_EXPLOSION_1 = 105,
    SFX_GROWL = 106,
    SFX_SPIDER_JUMP = 107,
    SFX_MENU_ROTATE = 108,
    SFX_MENU_LARA_HOME = 109,
    SFX_MENU_SPININ = 111,
    SFX_MENU_SPINOUT = 112,
    SFX_MENU_STOPWATCH = 113,
    SFX_MENU_GUNS = 114,
    SFX_MENU_PASSPORT = 115,
    SFX_MENU_MEDI = 116,
    SFX_ENEMY_HEELS = 117,
    SFX_ENEMY_FIRE_SILENCER = 118,
    SFX_ENEMY_AH_DYING = 119,
    SFX_ENEMY_OOH_DYING = 120,
    SFX_ENEMY_THUMP = 121,
    SFX_SPIDER_MOVING = 122,
    SFX_LARA_MINI_LOAD = 123,
    SFX_LARA_MINI_LOCK = 124,
    SFX_LARA_MINI_FIRE = 125,
    SFX_SPIDER_BITE = 126,
    SFX_SLAM_DOOR_SLIDE = 127,
    SFX_SLAM_DOOR_CLOSE = 128,
    SFX_EAGLE_SQUAWK = 129,
    SFX_EAGLE_WING_FLAP = 130,
    SFX_EAGLE_DEATH = 131,
    SFX_CROW_CAW = 132,
    SFX_CROW_WING_FLAP = 133,
    SFX_CROW_DEATH = 134,
    SFX_CROW_ATTACK = 135,
    SFX_ENEMY_GUN_COCKING = 136,
    SFX_ENEMY_FIRE_1 = 137,
    SFX_ENEMY_FIRE_TWIRL = 138,
    SFX_ENEMY_HOLSTER = 139,
    SFX_ENEMY_BREATH_1 = 140,
    SFX_ENEMY_CHUCKLE = 141,
    SFX_MONK_POY = 142,
    SFX_MONK_DEATH = 143,
    SFX_LARA_SPIKE_DEATH = 145,
    SFX_LARA_DEATH_3 = 146,
    SFX_ROLLING_BALL = 147,
    SFX_SANDBAG_SNAP = 148,
    SFX_SANDBAG_HIT = 149,
    SFX_LOOP_FOR_SMALL_FIRES = 150,
    SFX_SKIDOO_START = 152,
    SFX_SKIDOO_IDLE = 153,
    SFX_SKIDOO_ACCELERATE = 154,
    SFX_SKIDOO_MOVING = 155,
    SFX_SKIDOO_STOP = 156,
    SFX_ENEMY_FIRE_2 = 157,
    SFX_ENEMY_DEATH_2 = 158,
    SFX_ENEMY_BREATH_2 = 159,
    SFX_STICK_TAP = 160,
    SFX_TRAPDOOR_OPEN = 161,
    SFX_TRAPDOOR_CLOSE = 162,
    SFX_YETI_GROWL = 163,
    SFX_YETI_CHEST_BEAT = 164,
    SFX_YETI_THUMP = 165,
    SFX_YETI_GRUNT_1 = 166,
    SFX_YETI_SCREAM = 167,
    SFX_YETI_DEATH = 168,
    SFX_YETI_GROWL_1 = 169,
    SFX_YETI_GROWL_2 = 170,
    SFX_YETI_GRUNT_2 = 171,
    SFX_YETI_GROWL_3 = 172,
    SFX_YETI_FEET = 173,
    SFX_ENEMY_HEAVY_BREATH = 174,
    SFX_ENEMY_FLAMETHROWER_FIRE = 175,
    SFX_ENEMY_FLAMETHROWER_SCRAPE = 176,
    SFX_ENEMY_FLAMETHROWER_CLICK = 177,
    SFX_ENEMY_FLAMETHROWER_DEATH = 178,
    SFX_ENEMY_FLAMETHROWER_FALL = 179,
    SFX_ENEMY_BELT_JINGLE = 180,
    SFX_ENEMY_WRENCH = 181,
    SFX_FOOTSTEP = 182,
    SFX_FOOTSTEP_HIT = 183,
    SFX_ENEMY_COCKING_SHOTGUN = 184,
    SFX_SCUBA_DIVER_FLIPPER = 186,
    SFX_SCUBA_DIVER_BREATH = 188,
    SFX_PULLEY_CRANE = 190,
    SFX_CURTAIN = 191,
    SFX_SCUBA_DIVER_DEATH = 192,
    SFX_SCUBA_DIVER_DIVING = 193,
    SFX_BOAT_START = 194,
    SFX_BOAT_IDLE = 195,
    SFX_BOAT_ACCELERATE = 196,
    SFX_BOAT_MOVING = 197,
    SFX_BOAT_STOP = 198,
    SFX_BOAT_SLOW_DOWN = 199,
    SFX_BOAT_HIT = 200,
    SFX_CLATTER_1 = 201,
    SFX_CLATTER_2 = 202,
    SFX_CLATTER_3 = 203,
    SFX_DOOR_SLIDE = 204,
    SFX_LARA_FLESH_WOUND = 205,
    SFX_SAW_REVVING = 206,
    SFX_SAW_STOP = 207,
    SFX_DOOR_CHIME = 208,
    SFX_CHAIN_CREAK_SNAP = 209,
    SFX_SWINGING = 210,
    SFX_BREAKING_1 = 211,
    SFX_PULLEY_MOVE = 212,
    SFX_AIRPLANE_IDLE = 213,
    SFX_UNDERWATER_FAN_ON = 215,
    SFX_SMALL_FAN_ON = 217,
    SFX_SWINGING_BOX_BAG = 218,
    SFX_JUMP_PAD_UP = 219,
    SFX_JUMP_PAD_DOWN = 220,
    SFX_BREAKING_2 = 221,
    SFX_SNOWBALL_ROLL = 222,
    SFX_SNOWBALL_STOP = 223,
    SFX_ROLLING = 224,
    SFX_ROLLING_STOP_1 = 225,
    SFX_ROLLING_STOP_2 = 226,
    SFX_ROLLING_2 = 227,
    SFX_ROLLING_2_HIT = 228,
    SFX_SIDE_BLADE_SWING = 229,
    SFX_SIDE_BLADE_BACK = 230,
    SFX_ROLLING_BLADE = 231,
    SFX_ICILE_DETACH = 232,
    SFX_ICICLE_HIT = 233,
    SFX_ROTATING_HANDLE_LOOSE = 234,
    SFX_ROTATING_HANDLE_TURN = 235,
    SFX_ROTATING_HANDLE_OPEN = 236,
    SFX_ROTATING_HANDLE_CREAK = 237,
    SFX_MONK_FEET = 238,
    SFX_MONK_SWORD_SWING_1 = 239,
    SFX_MONK_SWORD_SWING_2 = 240,
    SFX_MONK_SHOUT_1 = 241,
    SFX_MONK_SHOUT_2 = 242,
    SFX_MONK_SHOUT_3 = 243,
    SFX_MONK_SHOUT_4 = 244,
    SFX_MONK_CRUNCH = 245,
    SFX_MONK_BREATH = 246,
    SFX_SPLASH_SURFACE = 247,
    SFX_WATERFALL_1 = 248,
    SFX_ENEMY_FEET_SNOW = 249,
    SFX_ENEMY_FIRE_3 = 250,
    SFX_ENEMY_FIRE_SEMIAUTO = 251,
    SFX_ENEMY_DEATH_3 = 252,
    SFX_ENEMY_DEATH_4 = 253,
    SFX_CIRCLE_BLADE = 254,
    SFX_KNIFETHROWER_FEET = 255,
    SFX_MONK_OYE = 256,
    SFX_MONK_AWEH = 257,
    SFX_CIRCLE_BLADE_HIT = 258,
    SFX_KNIFETHROWER_WARRIOR_FEET = 259,
    SFX_WARRIOR_BLADE_SWING_1 = 260,
    SFX_WARRIOR_BLADE_SWING_2 = 261,
    SFX_WARRIOR_GROWL = 262,
    SFX_KNIFETHROWER_HICCUP = 263,
    SFX_WARROPR_BURP = 264,
    SFX_WARRIOR_GROWL_1 = 265,
    SFX_WARRIOR_WAKE = 267,
    SFX_WARRIOR_GROWL_2 = 268,
    SFX_SMALL_SWITCH = 269,
    SFX_CHAIN_PULLEY = 278,
    SFX_ZIPLINE_GRAB = 279,
    SFX_ZIPLINE_GO = 280,
    SFX_ZIPLINE_STOP = 281,
    SFX_BODY_SLUMP = 282,
    SFX_BOWL_TIPPING = 283,
    SFX_BOWL_POUR = 284,
    SFX_WATERFALL_2 = 285,
    SFX_ELEVATOR_OPEN = 286,
    SFX_ELEVATOR_CLOSE = 287,
    SFX_MINISUB_CLATTER_1 = 288,
    SFX_MINISUB_CLATTER_2 = 289,
    SFX_MINISUB_CLATTER_3 = 290,
    SFX_BIRD_MONSTER_SCREAM = 291,
    SFX_BIRD_MONSTER_GASP = 292,
    SFX_BIRD_MONSTER_BREATH = 293,
    SFX_BIRD_MONSTER_FEET = 294,
    SFX_BIRD_MONSTER_DEATH = 295,
    SFX_BIRD_MONSTER_SCRAPE = 296,
    SFX_HELICOPTER_LOOP = 297,
    SFX_DRAGON_FEET = 298,
    SFX_DRAGON_GROWL_1 = 299,
    SFX_DRAGON_GROWL_2 = 300,
    SFX_DRAGON_FALL = 301,
    SFX_DRAGON_BREATH = 302,
    SFX_DRAGON_GROWL_3 = 303,
    SFX_DRAGON_GRUNT = 304,
    SFX_DRAGON_FIRE = 305,
    SFX_DRAGON_LEG_LIFT = 306,
    SFX_DRAGON_LEG_HIT = 307,
    SFX_WARRIOR_BLADE_SWING_3 = 308,
    SFX_WARRIOR_BLADE_SWING_FAST = 309,
    SFX_WARRIOR_BREATH_ACTIVE = 311,
    SFX_WARRIOR_HOVER = 312,
    SFX_WARRIOR_LANDING = 313,
    SFX_WARRIOR_SWORD_CLANK = 314,
    SFX_WARRIOR_SWORD_SLICE = 315,
    SFX_BIRDS_CHIRP = 316,
    SFX_CRUNCH_1 = 317,
    SFX_CRUNCH_2 = 318,
    SFX_DOOR_CREAK = 319,
    SFX_BREAKING_3 = 320,
    SFX_BIG_SPIDER_SNARL = 321,
    SFX_BIG_SPIDER_FEET = 322,
    SFX_BIG_SPIDER_DEATH = 323,
    SFX_T_REX_ROAR = 324,
    SFX_T_REX_FEET = 325,
    SFX_T_REX_GROWL_1 = 326,
    SFX_T_REX_DEATH = 327,
    SFX_DRIPS_REVERB = 329,
    SFX_STAGE_BACKDROP = 330,
    SFX_STONE_DOOR_SLIDE = 331,
    SFX_PLATFORM_ALARM = 332,
    SFX_TICK_TOCK = 333,
    SFX_DOORBELL = 334,
    SFX_BURGLAR_ALARM = 335,
    SFX_BOAT_ENGINE = 336,
    SFX_BOAT_INTO_WATER = 337,
    SFX_UNKNOWN_1 = 338,
    SFX_UNKNOWN_2 = 339,
    SFX_UNKNOWN_3 = 340,
    SFX_MARCO_BARTOLLI_TRANSFORM = 341,
    SFX_WINSTON_SHUFFLE = 342,
    SFX_WINSTON_FEET = 343,
    SFX_WINSTON_GRUNT_1 = 344,
    SFX_WINSTON_GRUNT_2 = 345,
    SFX_WINSTON_GRUNT_3 = 346,
    SFX_WINSTON_CUPS = 347,
    SFX_BRITTLE_GROUND_BREAK = 348,
    SFX_SPIDER_EXPLODE = 349,
    SFX_SHARK_BITE = 350,
    SFX_LAVA_BUBBLES = 351,
    SFX_EXPLOSION_2 = 352,
    SFX_BURGLARS = 353,
    SFX_ZIPPER = 354,
    SFX_NUMBER_OF = 370,
} SOUND_EFFECT_ID;

typedef enum {
    SPM_NORMAL     = 0,
    SPM_UNDERWATER = 1,
    SPM_ALWAYS     = 2,
    SPM_PITCH      = 4,
} SOUND_PLAY_MODE;

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
    MX_INACTIVE                = -1,
    MX_UNUSED_0                = 0, // 2.mp3
    MX_UNUSED_1                = 1, // 2.mp3
    MX_CUTSCENE_THE_GREAT_WALL = 2, // 2.mp3
    MX_UNUSED_2                = 3, // 2.mp3
    MX_CUTSCENE_OPERA_HOUSE    = 4, // 3.mp3
    MX_CUTSCENE_BROTHER_CHAN   = 5, // 4.mp3
    MX_GYM_HINT_1              = 6, // 5.mp3
    MX_GYM_HINT_2              = 7, // 6.mp3
    MX_GYM_HINT_3              = 8, // 7.mp3
    MX_GYM_HINT_4              = 9, // 8.mp3
    MX_GYM_HINT_5              = 10, // 9.mp3
    MX_GYM_HINT_6              = 11, // 10.mp3
    MX_GYM_HINT_7              = 12, // 11.mp3
    MX_GYM_HINT_8              = 13, // 12.mp3
    MX_GYM_HINT_9              = 14, // 13.mp3
    MX_GYM_HINT_10             = 15, // 14.mp3
    MX_GYM_HINT_11             = 16, // 15.mp3
    MX_GYM_HINT_12             = 17, // 16.mp3
    MX_GYM_HINT_13             = 18, // 17.mp3
    MX_GYM_HINT_14             = 19, // 18.mp3
    MX_UNUSED_3                = 20, // 18.mp3
    MX_UNUSED_4                = 21, // 18.mp3
    MX_GYM_HINT_15             = 22, // 19.mp3
    MX_GYM_HINT_16             = 23, // 20.mp3
    MX_GYM_HINT_17             = 24, // 21.mp3
    MX_GYM_HINT_18             = 25, // 22.mp3
    MX_UNUSED_5                = 26, // 23.mp3
    MX_CUTSCENE_BATH           = 27, // 23.mp3
    MX_DAGGER_PULL             = 28, // 24.mp3
    MX_GYM_HINT_20             = 29, // 25.mp3
    MX_CUTSCENE_XIAN           = 30, // 26.mp3
    MX_CAVES_AMBIENCE          = 31, // 27.mp3
    MX_SEWERS_AMBIENCE         = 32, // 28.mp3
    MX_WINDY_AMBIENCE          = 33, // 29.mp3
    MX_HEARTBEAT_AMBIENCE      = 34, // 30.mp3
    MX_SURPRISE_1              = 35, // 31.mp3
    MX_SURPRISE_2              = 36, // 32.mp3
    MX_SURPRISE_3              = 37, // 33.mp3
    MX_OOH_AAH_1               = 38, // 34.mp3
    MX_OOH_AAH_2               = 39, // 35.mp3
    MX_VENICE_VIOLINS          = 40, // 36.mp3
    MX_END_OF_LEVEL            = 41, // 37.mp3
    MX_SPOOKY_1                = 42, // 38.mp3
    MX_SPOOKY_2                = 43, // 39.mp3
    MX_SPOOKY_3                = 44, // 40.mp3
    MX_HARP_THEME              = 45, // 41.mp3
    MX_MYSTERY_1               = 46, // 42.mp3
    MX_SECRET                  = 47, // 43.mp3
    MX_AMBUSH_1                = 48, // 44.mp3
    MX_AMBUSH_2                = 49, // 45.mp3
    MX_AMBUSH_3                = 50, // 46.mp3
    MX_AMBUSH_4                = 51, // 47.mp3
    MX_SKIDOO_THEME            = 52, // 48.mp3
    MX_BATTLE_THEME            = 53, // 49.mp3
    MX_MYSTERY_2               = 54, // 50.mp3
    MX_MYSTERY_3               = 55, // 51.mp3
    MX_MYSTERY_4               = 56, // 52.mp3
    MX_MYSTERY_5               = 57, // 53.mp3
    MX_RIG_AMBIENCE            = 58, // 54.mp3
    MX_TOMB_AMBIENCE           = 59, // 55.mp3
    MX_OOH_AAH_3               = 60, // 56.mp3
    MX_REVEAL_1                = 61, // 57.mp3
    MX_CUTSCENE_RIG            = 62, // 58.mp3
    MX_REVEAL_2                = 63, // 59.mp3
    MX_TITLE_THEME             = 64, // 60.mp3
    MX_UNUSED_6                = 65, // 61.mp3
} MUSIC_TRACK_ID;

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
    uint16_t x;
    uint16_t y;
} XGEN_X;

typedef struct __unaligned {
    int32_t x1;
    int32_t x2;
} XBUF_X;

typedef struct __unaligned {
    int16_t x;
    int16_t y;
    int16_t g;
} XGEN_XG;

typedef struct __unaligned {
    int32_t x1;
    int32_t g1;
    int32_t x2;
    int32_t g2;
} XBUF_XG;

typedef struct __unaligned {
    uint16_t x;
    uint16_t y;
    uint16_t g;
    uint16_t u;
    uint16_t v;
} XGEN_XGUV;

typedef struct __unaligned {
    int32_t x1;
    int32_t g1;
    int32_t u1;
    int32_t v1;
    int32_t x2;
    int32_t g2;
    int32_t u2;
    int32_t v2;
} XBUF_XGUV;

typedef struct __unaligned {
    uint16_t x;
    uint16_t y;
    uint16_t g;
    float rhw;
    float u;
    float v;
} XGEN_XGUVP;

typedef struct __unaligned {
    int32_t x1;
    int32_t g1;
    float u1;
    float v1;
    float rhw1;
    int32_t x2;
    int32_t g2;
    float u2;
    float v2;
    float rhw2;
} XBUF_XGUVP;

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
