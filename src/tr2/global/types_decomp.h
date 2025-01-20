// This file is autogenerated. To update it, run tools/generate_funcs.

#pragma once

#include "const.h"

#include <libtrx/game/camera/enum.h>
#include <libtrx/game/camera/types.h>
#include <libtrx/game/items.h>
#include <libtrx/game/math.h>
#include <libtrx/game/rooms/types.h>
#include <libtrx/game/text.h>
#include <libtrx/game/types.h>

#include <stdint.h>

// clang-format off
#pragma pack(push, 1)

typedef struct {
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

typedef struct {
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

typedef struct {
    uint8_t index[256];
} DEPTHQ_ENTRY;

typedef struct {
    uint8_t index[32];
} GOURAUD_ENTRY;

typedef struct {
    XYZ_32 pos;
    XYZ_16 rot;
} PHD_3DPOS;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t r;
} SPHERE;

typedef struct {
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

typedef struct {
    uint16_t u;
    uint16_t v;
} PHD_UV;

typedef struct {
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
} SPRITE_FLAG;

typedef struct {
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

typedef struct {
    uint32_t best_time[10];
    uint32_t best_finish[10];
    uint32_t finish_count;
} ASSAULT_STATS;

typedef struct {
    void *_0;
    int32_t _1;
} SORT_ITEM;

typedef enum {
    DRAW_OPAQUE    = 0,
    DRAW_COLOR_KEY = 1,
} DRAW_TYPE;

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
    uint8_t secret_flags;
} LEVEL_STATS;

typedef struct {
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

typedef struct {
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

typedef struct {
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

typedef struct {
    int16_t zone_num;
    int16_t enemy_zone_num;
    int32_t distance;
    int32_t ahead;
    int32_t bite;
    int16_t angle;
    int16_t enemy_facing;
} AI_INFO;

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

typedef struct {
    XYZ_32 pos;
    int32_t mesh_num;
} BITE;

typedef struct {
    SECTOR *sector;
    SECTOR old_sector;
    int16_t block;
} DOORPOS_DATA;

typedef struct {
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
    TRAP_SET      = 0,
    TRAP_ACTIVATE = 1,
    TRAP_WORKING  = 2,
    TRAP_FINISHED = 3,
} TRAP_ANIM;

typedef struct {
    uint16_t num_levels;
    uint16_t num_demos;
} GAME_FLOW_LEGACY;

typedef enum {
    CF_NORMAL        = 0,
    CF_FOLLOW_CENTRE = 1,
    CF_NO_CHUNKY     = 2,
    CF_CHASE_OBJECT  = 3,
} CAMERA_FLAGS;

typedef struct {
    int16_t tx;
    int16_t ty;
    int16_t tz;
    int16_t cx;
    int16_t cy;
    int16_t cz;
    int16_t fov;
    int16_t roll;
} CINE_FRAME;

typedef struct {
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

typedef struct {
    int16_t number;
    int16_t volume;
    int16_t randomness;
    int16_t flags;
} SAMPLE_INFO;

/*
typedef struct {
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

typedef struct {
    int32_t boat_turn;
    int32_t left_fallspeed;
    int32_t right_fallspeed;
    int16_t tilt_angle;
    int16_t extra_rotation;
    int32_t water;
    int32_t pitch;
} BOAT_INFO;

typedef struct {
    int16_t track_mesh;
    int32_t skidoo_turn;
    int32_t left_fallspeed;
    int32_t right_fallspeed;
    int16_t momentum_angle;
    int16_t extra_rotation;
    int32_t pitch;
} SKIDOO_INFO;

typedef struct {
    struct {
        XYZ_16 min;
        XYZ_16 max;
    } shift, rot;
} OBJECT_BOUNDS;

typedef struct {
    int32_t xv;
    int32_t yv;
    int32_t zv;
} PORTAL_VBUF;

typedef struct {
    int32_t table[32]; // WIBBLE_SIZE
} ROOM_LIGHT_TABLE;

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

#pragma pack(pop)

typedef struct {
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

// clang-format on
