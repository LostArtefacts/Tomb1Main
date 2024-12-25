#pragma once

#include "global/types.h"

#include <stdint.h>

void Boat_Initialise(int16_t item_num);
void Boat_Setup(void);
int32_t Boat_CheckGetOn(int16_t item_num, const COLL_INFO *coll);
void Boat_Collision(int16_t item_num, ITEM *lara, COLL_INFO *coll);
int32_t Boat_TestWaterHeight(
    const ITEM *item, int32_t z_off, int32_t x_off, XYZ_32 *pos);
void Boat_DoShift(int32_t boat_num);
void Boat_DoWakeEffect(const ITEM *boat);
int32_t Boat_DoDynamics(int32_t height, int32_t fall_speed, int32_t *y);
int32_t Boat_Dynamics(int16_t boat_num);
int32_t Boat_UserControl(ITEM *boat);
void Boat_Animation(const ITEM *boat, int32_t collide);
void Boat_Control(int16_t item_num);
