#pragma once

#include "global/types.h"

#include <libtrx/game/text.h>

#define TEXT_HEIGHT 15

int32_t __cdecl Text_GetHeight(const TEXTSTRING *text);

void __cdecl Text_DrawBorder(
    int32_t x, int32_t y, int32_t z, int32_t width, int32_t height);
void __cdecl Text_DrawText(TEXTSTRING *text);

int32_t __cdecl Text_GetScaleH(uint32_t value);
int32_t __cdecl Text_GetScaleV(uint32_t value);
