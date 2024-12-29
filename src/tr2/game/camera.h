#pragma once

#include "global/types.h"

void Camera_Initialise(void);
void Camera_ResetPosition(void);
void Camera_Move(const GAME_VECTOR *target, int32_t speed);
void Camera_Clip(
    int32_t *x, int32_t *y, int32_t *h, int32_t target_x, int32_t target_y,
    int32_t target_h, int32_t left, int32_t top, int32_t right, int32_t bottom);
void Camera_Shift(
    int32_t *x, int32_t *y, int32_t *h, int32_t target_x, int32_t target_y,
    int32_t target_h, int32_t left, int32_t top, int32_t right, int32_t bottom);
const SECTOR *Camera_GoodPosition(
    int32_t x, int32_t y, int32_t z, int16_t room_num);
void Camera_SmartShift(
    GAME_VECTOR *target,
    void (*shift)(
        int32_t *x, int32_t *y, int32_t *h, int32_t target_x, int32_t target_y,
        int32_t target_h, int32_t left, int32_t top, int32_t right,
        int32_t bottom));
void Camera_Chase(const ITEM *item);
int32_t Camera_ShiftClamp(GAME_VECTOR *pos, int32_t clamp);
void Camera_Combat(const ITEM *item);
void Camera_Look(const ITEM *item);
void Camera_Fixed(void);
void Camera_Update(void);
void Camera_LoadCutsceneFrame(void);
void Camera_UpdateCutscene(void);
void Camera_RefreshFromTrigger(const TRIGGER *trigger);
