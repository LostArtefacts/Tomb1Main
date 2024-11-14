#pragma once

#include "global/types.h"

void __cdecl FallingBlock_Control(int16_t item_num);
void __cdecl FallingBlock_Floor(
    const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_height);
void __cdecl FallingBlock_Ceiling(
    const ITEM *item, int32_t x, int32_t y, int32_t z, int32_t *out_height);
