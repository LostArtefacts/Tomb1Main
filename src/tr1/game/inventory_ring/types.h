#pragma once

#include <libtrx/game/objects/types.h>

#include <stdint.h>

typedef enum {
    RT_MAIN = 0,
    RT_OPTION = 1,
    RT_KEYS = 2,
} RING_TYPE;

typedef struct {
    int16_t shape;
    int16_t x;
    int16_t y;
    int16_t z;
    int32_t param1;
    int32_t param2;
    int16_t sprnum;
} INVENTORY_SPRITE;

typedef enum {
    ACTION_USE = 0,
    ACTION_EXAMINE = 1,
} INVENTORY_ITEM_ACTION;

typedef struct {
    GAME_OBJECT_ID object_id;
    int16_t frames_total;
    int16_t current_frame;
    int16_t goal_frame;
    int16_t open_frame;
    int16_t anim_direction;
    int16_t pt_xrot_sel;
    int16_t pt_xrot;
    int16_t x_rot_sel;
    int16_t x_rot;
    int16_t y_rot_sel;
    int16_t y_rot;
    int32_t ytrans_sel;
    int32_t ytrans;
    int32_t ztrans_sel;
    int32_t ztrans;
    uint32_t which_meshes;
    uint32_t drawn_meshes;
    int16_t inv_pos;
    INVENTORY_SPRITE **sprlist;
    INVENTORY_ITEM_ACTION action;
} INVENTORY_ITEM;

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
    int16_t count;
    int16_t status;
    int16_t status_target;
    int16_t radius_target;
    int16_t radius_rate;
    int16_t camera_ytarget;
    int16_t camera_yrate;
    int16_t camera_pitch_target;
    int16_t camera_pitch_rate;
    int16_t rotate_target;
    int16_t rotate_rate;
    int16_t item_ptxrot_target;
    int16_t item_ptxrot_rate;
    int16_t item_xrot_target;
    int16_t item_xrot_rate;
    int32_t item_ytrans_target;
    int32_t item_ytrans_rate;
    int32_t item_ztrans_target;
    int32_t item_ztrans_rate;
    int32_t misc;
} IMOTION_INFO;

typedef struct {
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
    struct {
        XYZ_32 pos;
        XYZ_16 rot;
    } ringpos;
    struct {
        XYZ_32 pos;
        XYZ_16 rot;
    } camera;
    XYZ_32 light;
    IMOTION_INFO *imo;
} RING_INFO;
