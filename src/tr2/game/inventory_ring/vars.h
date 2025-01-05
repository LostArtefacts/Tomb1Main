#pragma once

#include "game/inventory_ring/types.h"
#include "global/types.h"

extern int32_t g_Inv_OptionObjectsCount;
extern INV_ITEM *g_Inv_OptionList[];

extern int16_t g_Inv_MainObjectsCount;
extern int16_t g_Inv_Chosen;
extern INVENTORY_MODE g_Inv_Mode;
extern uint16_t g_Inv_MainCurrent;
extern uint16_t g_Inv_KeyObjectsCount;
extern uint16_t g_Inv_KeysCurrent;
extern uint16_t g_Inv_OptionCurrent;
extern TEXTSTRING *g_Inv_RingText;
extern TEXTSTRING *g_Inv_UpArrow1;
extern TEXTSTRING *g_Inv_UpArrow2;
extern TEXTSTRING *g_Inv_DownArrow1;
extern TEXTSTRING *g_Inv_DownArrow2;
extern bool g_Inv_DemoMode;
extern bool g_Inv_IsOptionsDelay;
extern int32_t g_Inv_OptionsDelayCounter;
extern int32_t g_Inv_ExtraData[8];
extern INV_ITEM *g_Inv_MainList[];
extern INV_ITEM *g_Inv_KeysList[];
extern TEXTSTRING *g_Inv_ItemText[IT_NUMBER_OF];

extern INV_ITEM g_Inv_Item_Stopwatch;
extern INV_ITEM g_Inv_Item_Pistols;
extern INV_ITEM g_Inv_Item_Flare;
extern INV_ITEM g_Inv_Item_Shotgun;
extern INV_ITEM g_Inv_Item_Magnums;
extern INV_ITEM g_Inv_Item_Uzis;
extern INV_ITEM g_Inv_Item_Harpoon;
extern INV_ITEM g_Inv_Item_M16;
extern INV_ITEM g_Inv_Item_Grenade;
extern INV_ITEM g_Inv_Item_PistolAmmo;
extern INV_ITEM g_Inv_Item_ShotgunAmmo;
extern INV_ITEM g_Inv_Item_MagnumAmmo;
extern INV_ITEM g_Inv_Item_UziAmmo;
extern INV_ITEM g_Inv_Item_HarpoonAmmo;
extern INV_ITEM g_Inv_Item_M16Ammo;
extern INV_ITEM g_Inv_Item_GrenadeAmmo;
extern INV_ITEM g_Inv_Item_SmallMedi;
extern INV_ITEM g_Inv_Item_LargeMedi;
extern INV_ITEM g_Inv_Item_Pickup1;
extern INV_ITEM g_Inv_Item_Pickup2;
extern INV_ITEM g_Inv_Item_Puzzle1;
extern INV_ITEM g_Inv_Item_Puzzle2;
extern INV_ITEM g_Inv_Item_Puzzle3;
extern INV_ITEM g_Inv_Item_Puzzle4;
extern INV_ITEM g_Inv_Item_Key1;
extern INV_ITEM g_Inv_Item_Key2;
extern INV_ITEM g_Inv_Item_Key3;
extern INV_ITEM g_Inv_Item_Key4;
extern INV_ITEM g_Inv_Item_Passport;
extern INV_ITEM g_Inv_Item_Graphics;
extern INV_ITEM g_Inv_Item_Sound;
extern INV_ITEM g_Inv_Item_Controls;
extern INV_ITEM g_Inv_Item_Photo;
