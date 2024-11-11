#pragma once

#include "global/types.h"

typedef enum {
    LF_FL_HOLD_FT = 1,
    LF_FL_THROW_FT = 32,
    LF_FL_DRAW_FT = 39,
    LF_FL_IGNITE_FT = 23,
    LF_FL_2_HOLD_FT = 15,

    LF_FL_HOLD = 0,
    LF_FL_THROW = (LF_FL_HOLD + LF_FL_HOLD_FT), // = 1
    LF_FL_DRAW = (LF_FL_THROW + LF_FL_THROW_FT), // = 33
    LF_FL_IGNITE = (LF_FL_DRAW + LF_FL_DRAW_FT), // = 72
    LF_FL_2_HOLD = (LF_FL_IGNITE + LF_FL_IGNITE_FT), // = 95
    LF_FL_END = (LF_FL_2_HOLD + LF_FL_2_HOLD_FT), // = 110
} LARA_FLARE_ANIMATION_FRAME;

int32_t __cdecl Flare_DoLight(const XYZ_32 *pos, int32_t flare_age);
void __cdecl Flare_DoInHand(int32_t flare_age);
void __cdecl Flare_DrawInAir(const ITEM *item);
void __cdecl Flare_Create(bool thrown);
void __cdecl Flare_SetArm(int32_t frame);
