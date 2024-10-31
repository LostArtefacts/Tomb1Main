#pragma once

#include "global/types.h"

#include <libtrx/game/text.h>

#define TEXT_HEIGHT 15

void __cdecl Text_RemoveOutline(TEXTSTRING *text);
void __cdecl Text_CentreH(TEXTSTRING *text, int16_t enable);
void __cdecl Text_CentreV(TEXTSTRING *text, int16_t enable);
void __cdecl Text_AlignRight(TEXTSTRING *text, int16_t enable);
void __cdecl Text_AlignBottom(TEXTSTRING *text, int16_t enable);
void __cdecl Text_SetMultiline(TEXTSTRING *textstring, bool enable);
int32_t __cdecl Text_Remove(TEXTSTRING *text);
int32_t __cdecl Text_GetWidth(TEXTSTRING *text);
int32_t __cdecl Text_GetHeight(const TEXTSTRING *text);

void __cdecl Text_DrawBorder(
    int32_t x, int32_t y, int32_t z, int32_t width, int32_t height);
void __cdecl Text_DrawText(TEXTSTRING *text);
int32_t __cdecl Text_GetScaleH(uint32_t value);
int32_t __cdecl Text_GetScaleV(uint32_t value);
