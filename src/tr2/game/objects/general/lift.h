#pragma once

#include "global/types.h"

typedef struct {
    int32_t start_height;
    int32_t wait_time;
} LIFT_INFO;

void Lift_Setup(void);
void Lift_Initialise(int16_t item_num);
void Lift_Control(int16_t item_num);
void Lift_FloorCeiling(
    const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_floor,
    int32_t *out_ceiling);
void Lift_Floor(
    const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_height);
void Lift_Ceiling(
    const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_height);
