#pragma once

#include "game/items.h"

int32_t __cdecl Effect_ExplodingDeath(
    int16_t item_num, int32_t mesh_bits, int16_t damage);

int16_t __cdecl Effect_MissileFlame(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

void __cdecl Effect_CreateBartoliLight(int16_t item_num);

void __cdecl FX_AssaultStart(ITEM *item);

void __cdecl CreateBubble(const XYZ_32 *pos, int16_t room_num);
void __cdecl FX_Bubbles(ITEM *item);

void __cdecl Splash(const ITEM *item);

void __cdecl FX_LaraHandsFree(ITEM *item);

int16_t __cdecl Effect_GunShot(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot,
    int16_t room_num);

int16_t __cdecl Effect_GunHit(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

int16_t __cdecl Effect_GunMiss(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

int16_t __cdecl Knife(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

int16_t __cdecl DoBloodSplat(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t direction,
    int16_t room_num);

void __cdecl DoLotsOfBlood(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t direction,
    int16_t room_num, int32_t num);

void __cdecl Richochet(const GAME_VECTOR *pos);

void __cdecl FX_FinishLevel(ITEM *item);
void __cdecl FX_Turn180(ITEM *item);
void __cdecl FX_FloorShake(ITEM *item);
void __cdecl FX_LaraNormal(ITEM *item);
void __cdecl FX_Boiler(ITEM *item);
void __cdecl FX_Flood(ITEM *item);
void __cdecl FX_Rubble(ITEM *item);
void __cdecl FX_Chandelier(ITEM *item);
void __cdecl FX_Explosion(ITEM *item);
void __cdecl FX_Piston(ITEM *item);
void __cdecl FX_Curtain(ITEM *item);
void __cdecl FX_Statue(ITEM *item);
void __cdecl FX_SetChange(ITEM *item);
void __cdecl FX_FlipMap(ITEM *item);
void __cdecl FX_LaraDrawRightGun(ITEM *item);
void __cdecl FX_LaraDrawLeftGun(ITEM *item);
void __cdecl FX_SwapMeshesWithMeshSwap1(ITEM *item);
void __cdecl FX_SwapMeshesWithMeshSwap2(ITEM *item);
void __cdecl FX_SwapMeshesWithMeshSwap3(ITEM *item);
