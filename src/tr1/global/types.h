#pragma once

#include "game/stats/types.h"
#include "global/const.h"

#include <libtrx/game/anims.h>
#include <libtrx/game/camera/enum.h>
#include <libtrx/game/camera/types.h>
#include <libtrx/game/collision.h>
#include <libtrx/game/creature.h>
#include <libtrx/game/effects/types.h>
#include <libtrx/game/game_flow/enum.h>
#include <libtrx/game/game_flow/types.h>
#include <libtrx/game/items.h>
#include <libtrx/game/lara/types.h>
#include <libtrx/game/lot.h>
#include <libtrx/game/math.h>
#include <libtrx/game/matrix.h>
#include <libtrx/game/objects/common.h>
#include <libtrx/game/rooms.h>
#include <libtrx/game/sound/enum.h>
#include <libtrx/game/sound/ids.h>
#include <libtrx/game/text.h>
#include <libtrx/game/types.h>

#include <stddef.h>
#include <stdint.h>

typedef int16_t PHD_ANGLE;

typedef enum {
    SAMPLE_FLAG_NO_PAN = 1 << 12,
    SAMPLE_FLAG_PITCH_WIBBLE = 1 << 13,
    SAMPLE_FLAG_VOLUME_WIBBLE = 1 << 14,
} SAMPLE_FLAG;

typedef enum {
    MX_INACTIVE = -1,
    MX_UNUSED_0 = 0,
    MX_UNUSED_1 = 1,
    MX_TR_THEME = 2,
    MX_WHERE_THE_DEPTHS_UNFOLD_1 = 3,
    MX_TR_THEME_ALT_1 = 4,
    MX_UNUSED_2 = 5,
    MX_TIME_TO_RUN_1 = 6,
    MX_FRIEND_SINCE_GONE = 7,
    MX_T_REX_1 = 8,
    MX_A_LONG_WAY_DOWN = 9,
    MX_LONGING_FOR_HOME = 10,
    MX_SPOOKY_1 = 11,
    MX_KEEP_YOUR_BALANCE = 12,
    MX_SECRET = 13,
    MX_SPOOKY_3 = 14,
    MX_WHERE_THE_DEPTHS_UNFOLD_2 = 15,
    MX_T_REX_2 = 16,
    MX_WHERE_THE_DEPTHS_UNFOLD_3 = 17,
    MX_WHERE_THE_DEPTHS_UNFOLD_4 = 18,
    MX_TR_THEME_ALT_2 = 19,
    MX_TIME_TO_RUN_2 = 20,
    MX_LONGING_FOR_HOME_ALT = 21,
    MX_NATLA_FALLS_CUTSCENE = 22,
    MX_LARSON_CUTSCENE = 23,
    MX_NATLA_PLACES_SCION_CUTSCENE = 24,
    MX_LARA_TIHOCAN_CUTSCENE = 25,
    MX_GYM_HINT_01 = 26,
    MX_GYM_HINT_02 = 27,
    MX_GYM_HINT_03 = 28,
    MX_GYM_HINT_04 = 29,
    MX_GYM_HINT_05 = 30,
    MX_GYM_HINT_06 = 31,
    MX_GYM_HINT_07 = 32,
    MX_GYM_HINT_08 = 33,
    MX_GYM_HINT_09 = 34,
    MX_GYM_HINT_10 = 35,
    MX_GYM_HINT_11 = 36,
    MX_GYM_HINT_12 = 37,
    MX_GYM_HINT_13 = 38,
    MX_GYM_HINT_14 = 39,
    MX_GYM_HINT_15 = 40,
    MX_GYM_HINT_16 = 41,
    MX_GYM_HINT_17 = 42,
    MX_GYM_HINT_18 = 43,
    MX_GYM_HINT_19 = 44,
    MX_GYM_HINT_20 = 45,
    MX_GYM_HINT_21 = 46,
    MX_GYM_HINT_22 = 47,
    MX_GYM_HINT_23 = 48,
    MX_GYM_HINT_24 = 49,
    MX_GYM_HINT_25 = 50,
    MX_BALDY_SPEECH = 51,
    MX_COWBOY_SPEECH = 52,
    MX_LARSON_SPEECH = 53,
    MX_NATLA_SPEECH = 54,
    MX_PIERRE_SPEECH = 55,
    MX_SKATEKID_SPEECH = 56,
    MX_CAVES_AMBIENCE = 57,
    MX_CISTERN_AMBIENCE = 58,
    MX_WINDY_AMBIENCE = 59,
    MX_ATLANTIS_AMBIENCE = 60,
    MX_NUMBER_OF,
} MUSIC_TRACK_ID;

typedef enum {
    TARGET_NONE = 0,
    TARGET_PRIMARY = 1,
    TARGET_SECONDARY = 2,
} TARGET_TYPE;

typedef enum {
    D_TRANS1 = 1,
    D_TRANS2 = 2,
    D_TRANS3 = 3,
    D_TRANS4 = 4,
    D_NEXT = 1 << 3,
} D_FLAGS;

typedef enum {
    COLL_NONE = 0,
    COLL_FRONT = 1,
    COLL_LEFT = 2,
    COLL_RIGHT = 4,
    COLL_TOP = 8,
    COLL_TOPFRONT = 16,
    COLL_CLAMP = 32,
} COLL_TYPE;

typedef enum {
    HT_WALL = 0,
    HT_SMALL_SLOPE = 1,
    HT_BIG_SLOPE = 2,
} HEIGHT_TYPE;

typedef enum {
    IC_BLACK = 0,
    IC_GREY = 1,
    IC_WHITE = 2,
    IC_RED = 3,
    IC_ORANGE = 4,
    IC_YELLOW = 5,
    IC_GREEN1 = 6,
    IC_GREEN2 = 7,
    IC_GREEN3 = 8,
    IC_GREEN4 = 9,
    IC_GREEN5 = 10,
    IC_GREEN6 = 11,
    IC_DARKGREEN = 12,
    IC_GREEN = 13,
    IC_CYAN = 14,
    IC_BLUE = 15,
    IC_MAGENTA = 16,
    IC_NUMBER_OF = 17,
} INV_COLOUR;

typedef enum {
    SHAPE_SPRITE = 1,
    SHAPE_LINE = 2,
    SHAPE_BOX = 3,
    SHAPE_FBOX = 4
} SHAPE;

typedef enum {
    TRAP_SET = 0,
    TRAP_ACTIVATE = 1,
    TRAP_WORKING = 2,
    TRAP_FINISHED = 3,
} TRAP_ANIM;

typedef enum {
    SPS_START = 0,
    SPS_END = 1,
    SPS_MOVING = 2,
} SLIDING_PILLAR_STATE;

typedef enum {
    GFS_END = -1,
    GFS_START_GAME,
    GFS_LOOP_GAME,
    GFS_STOP_GAME,
    GFS_START_CINE,
    GFS_LOOP_CINE,
    GFS_PLAY_FMV,
    GFS_LEVEL_STATS,
    GFS_TOTAL_STATS,
    GFS_LOADING_SCREEN,
    GFS_DISPLAY_PICTURE,
    GFS_EXIT_TO_TITLE,
    GFS_EXIT_TO_LEVEL,
    GFS_EXIT_TO_CINE,
    GFS_SET_CAM_X,
    GFS_SET_CAM_Y,
    GFS_SET_CAM_Z,
    GFS_SET_CAM_ANGLE,
    GFS_FLIP_MAP,
    GFS_REMOVE_GUNS,
    GFS_REMOVE_SCIONS,
    GFS_GIVE_ITEM,
    GFS_PLAY_SYNCED_AUDIO,
    GFS_MESH_SWAP,
    GFS_REMOVE_AMMO,
    GFS_REMOVE_MEDIPACKS,
    GFS_SETUP_BACON_LARA,
    GFS_LEGACY,
} GAME_FLOW_SEQUENCE_TYPE;

typedef enum {
    BT_LARA_HEALTH = 0,
    BT_LARA_MAX_AIR = 1,
    BT_ENEMY_HEALTH = 2,
    BT_PROGRESS = 3,
} BAR_TYPE;

typedef enum {
    GBF_NGPLUS = 1 << 0,
    GBF_JAPANESE = 1 << 1,
} GAME_BONUS_FLAG;

typedef enum {
    PAGE_1 = 0,
    PAGE_2 = 1,
    PAGE_3 = 2,
    PAGE_COUNT = 3,
} PASSPORT_PAGE;

typedef enum {
    PASSPORT_MODE_BROWSE = 0,
    PASSPORT_MODE_LOAD_GAME = 1,
    PASSPORT_MODE_SELECT_LEVEL = 2,
    PASSPORT_MODE_STORY_SO_FAR = 3,
    PASSPORT_MODE_SAVE_GAME = 4,
    PASSPORT_MODE_NEW_GAME = 5,
    PASSPORT_MODE_RESTART = 6,
    PASSPORT_MODE_EXIT_TITLE = 7,
    PASSPORT_MODE_EXIT_GAME = 8,
    PASSPORT_MODE_UNAVAILABLE = 9,
} PASSPORT_MODE;

typedef struct {
    int width;
    int height;
} RESOLUTION;

typedef struct {
    float r;
    float g;
    float b;
} RGB_F;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_888;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA_8888;

typedef struct {
    float xv;
    float yv;
    float zv;
    float xs;
    float ys;
    float u;
    float v;
    float g;
} POINT_INFO;

typedef struct {
    float xv;
    float yv;
    float zv;
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
    uint16_t drawtype;
    uint16_t tpage;
    PHD_UV uv[4];
} PHD_TEXTURE;

typedef struct {
    uint16_t tpage;
    uint16_t offset;
    uint16_t width;
    uint16_t height;
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} PHD_SPRITE;

typedef struct TEXTURE_RANGE {
    int16_t num_textures;
    int16_t *textures;
    struct TEXTURE_RANGE *next_range;
} TEXTURE_RANGE;

typedef struct {
    SECTOR *sector;
    SECTOR old_sector;
    int16_t block;
} DOORPOS_DATA;

typedef struct {
    int16_t tx;
    int16_t ty;
    int16_t tz;
    int16_t cx;
    int16_t cy;
    int16_t cz;
    int16_t fov;
    int16_t roll;
} CINE_CAMERA;

typedef struct {
    XYZ_32 pos;
    int16_t rot;
} CINE_POSITION;

typedef struct {
    int32_t lara_hitpoints;
    uint16_t pistol_ammo;
    uint16_t magnum_ammo;
    uint16_t uzi_ammo;
    uint16_t shotgun_ammo;
    uint8_t num_medis;
    uint8_t num_big_medis;
    uint8_t num_scions;
    int8_t gun_status;
    LARA_GUN_TYPE equipped_gun_type;
    LARA_GUN_TYPE holsters_gun_type;
    LARA_GUN_TYPE back_gun_type;
    union {
        uint16_t all;
        struct {
            uint16_t available : 1;
            uint16_t got_pistols : 1;
            uint16_t got_magnums : 1;
            uint16_t got_uzis : 1;
            uint16_t got_shotgun : 1;
            uint16_t costume : 1;
        };
    } flags;
    LEVEL_STATS stats;
} RESUME_INFO;

typedef struct {
    RESUME_INFO *current;
    uint8_t bonus_flag;
    bool bonus_level_unlock;
    int32_t current_save_slot;
    int16_t save_initial_version;
    PASSPORT_MODE passport_selection;
    int32_t select_level_num;
    bool death_counter_supported;
    GAME_FLOW_LEVEL_TYPE current_level_type;
    GAME_FLOW_COMMAND override_gf_command;
    bool remove_guns;
    bool remove_scions;
    bool remove_ammo;
    bool remove_medipacks;

    // TODO: remove these from here and make it a responsibility of overlay.c
    bool inv_ring_shown;
    bool showing_demo;
    bool inv_showing_medpack;
    bool inv_ring_above;

    bool ask_for_save;
} GAME_INFO;

typedef enum {
    MC_PURPLE_C,
    MC_PURPLE_E,
    MC_BROWN_C,
    MC_BROWN_E,
    MC_GREY_C,
    MC_GREY_E,
    MC_GREY_TL,
    MC_GREY_TR,
    MC_GREY_BL,
    MC_GREY_BR,
    MC_BLACK,
    MC_GOLD_LIGHT,
    MC_GOLD_DARK,
    MC_NUMBER_OF,
} MENU_COLOR;

typedef struct {
    int32_t xv;
    int32_t yv;
    int32_t zv;
} DOOR_VBUF;

typedef struct {
    PHD_ANGLE lock_angles[4];
    PHD_ANGLE left_angles[4];
    PHD_ANGLE right_angles[4];
    PHD_ANGLE aim_speed;
    PHD_ANGLE shot_accuracy;
    int32_t gun_height;
    int16_t damage;
    int32_t target_dist;
    int16_t recoil_frame;
    int16_t flash_time;
    int16_t sample_num;
} WEAPON_INFO;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t r;
} SPHERE;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t mesh_num;
} BITE;

typedef struct {
    int16_t zone_num;
    int16_t enemy_zone;
    int32_t distance;
    int32_t ahead;
    int32_t bite;
    int16_t angle;
    int16_t enemy_facing;
} AI_INFO;

typedef struct {
    int32_t left;
    int32_t right;
    int32_t top;
    int32_t bottom;
    int16_t height;
    int16_t overlap_index;
} BOX_INFO;

typedef struct {
    bool is_blocked;
    char *content_text;
    TEXTSTRING *content;
} REQUESTER_ITEM;

typedef struct {
    uint16_t items_used;
    uint16_t max_items;
    uint16_t requested;
    uint16_t vis_lines;
    int16_t line_offset;
    int16_t line_old_offset;
    uint16_t pix_width;
    uint16_t line_height;
    bool is_blockable;
    int16_t x;
    int16_t y;
    char *heading_text;
    TEXTSTRING *heading;
    TEXTSTRING *background;
    TEXTSTRING *moreup;
    TEXTSTRING *moredown;
    REQUESTER_ITEM *items;
} REQUEST_INFO;

typedef struct {
    int16_t number;
    int16_t volume;
    int16_t randomness;
    int16_t flags;
} SAMPLE_INFO;

typedef struct {
    int32_t mesh_count;
    int32_t mesh_ptr_count;
    int32_t anim_count;
    int32_t anim_change_count;
    int32_t anim_range_count;
    int32_t anim_command_count;
    int32_t anim_bone_count;
    int32_t anim_frame_data_count;
    int16_t *anim_frame_data;
    int32_t static_count;
    int32_t texture_count;
    int32_t texture_page_count;
    uint8_t *texture_palette_page_ptrs;
    RGBA_8888 *texture_rgb_page_ptrs;
    int32_t anim_texture_range_count;
    int32_t item_count;
    int32_t sprite_info_count;
    int32_t sprite_count;
    int32_t overlap_count;
    int32_t sample_info_count;
    int32_t sample_count;
    int32_t *sample_offsets;
    int32_t sample_data_size;
    char *sample_data;
    RGB_888 *palette;
    int32_t palette_size;
} LEVEL_INFO;
