#pragma once

#include "global/types.h"

int32_t __cdecl Flare_DoLight(const XYZ_32 *pos, int32_t flare_age);
void __cdecl Flare_DoInHand(int32_t flare_age);
void __cdecl Flare_DrawInAir(const ITEM *item);
