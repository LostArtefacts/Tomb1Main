#pragma once

#include "global/types.h"

void Detonator1_Setup(void);
void Detonator2_Setup(void);

void Detonator_Control(int16_t item_num);

void Detonator_Collision(int16_t item_num, ITEM *lara_item, COLL_INFO *coll);
