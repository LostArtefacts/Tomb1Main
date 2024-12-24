#pragma once

#include "game/items.h"

int16_t __cdecl Spawn_FireStream(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

void __cdecl Spawn_MysticLight(int16_t item_num);

void __cdecl Spawn_Bubble(const XYZ_32 *pos, int16_t room_num);

void __cdecl Spawn_Splash(const ITEM *item);

int16_t __cdecl Spawn_GunShot(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot,
    int16_t room_num);

int16_t __cdecl Spawn_GunHit(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

int16_t __cdecl Spawn_GunMiss(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

int16_t __cdecl Spawn_Knife(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

int16_t __cdecl Spawn_Blood(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num);

void __cdecl Spawn_BloodBath(
    int32_t x, int32_t y, int32_t z, int16_t speed, int16_t y_rot,
    int16_t room_num, int32_t count);

void __cdecl Spawn_Ricochet(const GAME_VECTOR *pos);
