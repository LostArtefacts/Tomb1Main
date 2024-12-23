#pragma once

#include "game/items.h"

int16_t __cdecl Effect_MissileFlame(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

void __cdecl Effect_CreateBartoliLight(int16_t item_num);

void __cdecl CreateBubble(const XYZ_32 *pos, int16_t room_num);

void __cdecl Splash(const ITEM *item);

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
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

void __cdecl DoLotsOfBlood(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num, int32_t count);

void __cdecl Ricochet(const GAME_VECTOR *pos);
