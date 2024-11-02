#pragma once

#include <stdint.h>

typedef struct {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t near_z;
    int32_t far_z;
    int16_t view_angle;
    int32_t screen_width;
    int32_t screen_height;
} VIEWPORT;

void Viewport_Init(
    int16_t x, int16_t y, int32_t width, int32_t height, int32_t near_z,
    int32_t far_z, int16_t view_angle, int32_t screen_width,
    int32_t screen_height);

void Viewport_AlterFOV(int16_t view_angle);

const VIEWPORT *Viewport_Get(void);

int16_t Viewport_GetFOV(void);
int16_t Viewport_GetUserFOV(void);
