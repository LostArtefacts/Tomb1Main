#pragma once

#include "../types.h"
#include "./enum.h"

typedef struct {
    GAME_VECTOR pos;
    GAME_VECTOR target;
    CAMERA_TYPE type;

    int32_t shift;
    uint32_t flags;
    int32_t fixed_camera;
    int32_t bounce;
    bool underwater;
    int32_t target_distance;
    int32_t target_square;
    int16_t target_angle;
#if TR_VERSION == 2
    int16_t actual_angle;
#endif
    int16_t target_elevation;
    int16_t num;
    int16_t last;
    int16_t timer;
    int16_t speed;
    int16_t roll;
    ITEM *item;
    ITEM *last_item;
    OBJECT_VECTOR *fixed;

    int32_t debuff;

#if TR_VERSION == 1
    // used for the manual camera control
    int16_t additional_angle;
    int16_t additional_elevation;

    struct {
        struct {
            XYZ_32 target;
            XYZ_32 pos;
            int32_t shift;
        } result, prev;
        int16_t room_num;
    } interp;
#else
    XYZ_32 mic_pos;
#endif
} CAMERA_INFO;
