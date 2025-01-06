#pragma once

#include "global/types.h"

typedef enum {
    INV_GAME_MODE = 0,
    INV_TITLE_MODE = 1,
    INV_KEYS_MODE = 2,
    INV_SAVE_MODE = 3,
    INV_LOAD_MODE = 4,
    INV_DEATH_MODE = 5,
} INVENTORY_MODE;

typedef enum {
    RT_MAIN = 0,
    RT_OPTION = 1,
    RT_KEYS = 2,
} RING_TYPE;

typedef enum {
    RNG_OPENING = 0,
    RNG_OPEN = 1,
    RNG_CLOSING = 2,
    RNG_MAIN2OPTION = 3,
    RNG_MAIN2KEYS = 4,
    RNG_KEYS2MAIN = 5,
    RNG_OPTION2MAIN = 6,
    RNG_SELECTING = 7,
    RNG_SELECTED = 8,
    RNG_DESELECTING = 9,
    RNG_DESELECT = 10,
    RNG_CLOSING_ITEM = 11,
    RNG_EXITING_INVENTORY = 12,
    RNG_DONE = 13,
} RING_STATUS;

typedef struct {
    int16_t shape;
    XYZ_16 pos;
    int32_t param1;
    int32_t param2;
    int16_t sprite_num;
} INVENTORY_SPRITE;

typedef struct {
    GAME_OBJECT_ID object_id;
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
} INVENTORY_ITEM;

typedef struct {
    int16_t count;
    RING_STATUS status;
    RING_STATUS status_target;
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
} INV_RING_MOTION;

typedef struct {
    INVENTORY_MODE mode;
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
    INV_RING_MOTION motion;

    bool is_demo_needed;
    bool is_pass_open;
    bool has_spun_out;
    int32_t old_fov;
} INV_RING;
