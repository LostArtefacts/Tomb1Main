#pragma once

#include "global/types.h"

#include <libtrx/game/text.h>

#define TEXT_HEIGHT 15

void Text_DrawBorder(
    int32_t x, int32_t y, int32_t z, int32_t width, int32_t height);
void Text_DrawText(TEXTSTRING *text);
