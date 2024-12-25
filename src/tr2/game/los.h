#pragma once

#include "global/types.h"

int32_t LOS_CheckX(const GAME_VECTOR *start, GAME_VECTOR *target);
int32_t LOS_CheckZ(const GAME_VECTOR *start, GAME_VECTOR *target);
int32_t LOS_ClipTarget(
    const GAME_VECTOR *start, GAME_VECTOR *target, const SECTOR *sector);
int32_t LOS_Check(const GAME_VECTOR *start, GAME_VECTOR *target);
int32_t LOS_CheckSmashable(const GAME_VECTOR *start, const GAME_VECTOR *target);
