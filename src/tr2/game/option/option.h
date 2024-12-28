#pragma once

#include "game/inventory_ring/types.h"

void Option_Control(INV_ITEM *item);
void Option_Draw(INV_ITEM *item);
void Option_Shutdown(INV_ITEM *item);

void Option_Passport_Control(INV_ITEM *item);
void Option_Passport_Draw(INV_ITEM *item);
void Option_Passport_Shutdown(void);

void Option_Detail_Control(INV_ITEM *item);
void Option_Detail_Draw(INV_ITEM *item);
void Option_Detail_Shutdown(void);

void Option_Sound_Control(INV_ITEM *item);
void Option_Sound_Draw(INV_ITEM *item);
void Option_Sound_Shutdown(void);

void Option_Controls_FlashConflicts(void);
void Option_Controls_DefaultConflict(void);
void Option_Controls_Control(INV_ITEM *item);
void Option_Controls_Draw(INV_ITEM *item);
void Option_Controls_Shutdown(void);
void Option_Controls_ShowControls(void);
void Option_Controls_UpdateText(void);

void Option_Compass_Control(INV_ITEM *item);
void Option_Compass_Draw(INV_ITEM *item);
void Option_Compass_Shutdown(void);
