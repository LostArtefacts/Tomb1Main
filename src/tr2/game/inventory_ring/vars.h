#pragma once

#include "global/types.h"

#include <libtrx/game/inventory_ring/types.h>

extern int32_t g_Inv_OptionObjectsCount;
extern INVENTORY_ITEM *g_Inv_OptionList[];

extern int16_t g_Inv_MainObjectsCount;
extern int16_t g_Inv_Chosen;
extern INVENTORY_MODE g_Inv_Mode;
extern uint16_t g_Inv_MainCurrent;
extern uint16_t g_Inv_KeyObjectsCount;
extern uint16_t g_Inv_KeysCurrent;
extern uint16_t g_Inv_OptionCurrent;
extern TEXTSTRING *g_InvRing_Text;
extern TEXTSTRING *g_Inv_UpArrow1;
extern TEXTSTRING *g_Inv_UpArrow2;
extern TEXTSTRING *g_Inv_DownArrow1;
extern TEXTSTRING *g_Inv_DownArrow2;
extern bool g_Inv_DemoMode;
extern bool g_Inv_IsOptionsDelay;
extern int32_t g_Inv_OptionsDelayCounter;
extern int32_t g_Inv_ExtraData[8];
extern INVENTORY_ITEM *g_Inv_MainList[];
extern INVENTORY_ITEM *g_Inv_KeysList[];
extern TEXTSTRING *g_Inv_ItemText[IT_NUMBER_OF];

extern INVENTORY_ITEM g_Inv_Item_Stopwatch;
extern INVENTORY_ITEM g_Inv_Item_Pistols;
extern INVENTORY_ITEM g_Inv_Item_Flare;
extern INVENTORY_ITEM g_Inv_Item_Shotgun;
extern INVENTORY_ITEM g_Inv_Item_Magnums;
extern INVENTORY_ITEM g_Inv_Item_Uzis;
extern INVENTORY_ITEM g_Inv_Item_Harpoon;
extern INVENTORY_ITEM g_Inv_Item_M16;
extern INVENTORY_ITEM g_Inv_Item_Grenade;
extern INVENTORY_ITEM g_Inv_Item_PistolAmmo;
extern INVENTORY_ITEM g_Inv_Item_ShotgunAmmo;
extern INVENTORY_ITEM g_Inv_Item_MagnumAmmo;
extern INVENTORY_ITEM g_Inv_Item_UziAmmo;
extern INVENTORY_ITEM g_Inv_Item_HarpoonAmmo;
extern INVENTORY_ITEM g_Inv_Item_M16Ammo;
extern INVENTORY_ITEM g_Inv_Item_GrenadeAmmo;
extern INVENTORY_ITEM g_Inv_Item_SmallMedi;
extern INVENTORY_ITEM g_Inv_Item_LargeMedi;
extern INVENTORY_ITEM g_Inv_Item_Pickup1;
extern INVENTORY_ITEM g_Inv_Item_Pickup2;
extern INVENTORY_ITEM g_Inv_Item_Puzzle1;
extern INVENTORY_ITEM g_Inv_Item_Puzzle2;
extern INVENTORY_ITEM g_Inv_Item_Puzzle3;
extern INVENTORY_ITEM g_Inv_Item_Puzzle4;
extern INVENTORY_ITEM g_Inv_Item_Key1;
extern INVENTORY_ITEM g_Inv_Item_Key2;
extern INVENTORY_ITEM g_Inv_Item_Key3;
extern INVENTORY_ITEM g_Inv_Item_Key4;
extern INVENTORY_ITEM g_Inv_Item_Passport;
extern INVENTORY_ITEM g_Inv_Item_Graphics;
extern INVENTORY_ITEM g_Inv_Item_Sound;
extern INVENTORY_ITEM g_Inv_Item_Controls;
extern INVENTORY_ITEM g_Inv_Item_Photo;
