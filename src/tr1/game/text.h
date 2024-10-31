#pragma once

#include "config.h"

#include <libtrx/game/text.h>

#include <stdbool.h>
#include <stdint.h>

#define TEXT_HEIGHT 11 // TODO: Get rid of this
#define TEXT_HEIGHT_FIXED 15
#define TEXT_OUTLINE_THICKNESS 2

RGBA_8888 Text_GetMenuColor(MENU_COLOR color);
void Text_CentreH(TEXTSTRING *text, bool enable);
void Text_CentreV(TEXTSTRING *text, bool enable);
void Text_AlignRight(TEXTSTRING *text, bool enable);
void Text_AlignBottom(TEXTSTRING *text, bool enable);
void Text_SetMultiline(TEXTSTRING *text, bool enable);
int32_t Text_GetWidth(const TEXTSTRING *text);
int32_t Text_GetHeight(const TEXTSTRING *text);
void Text_Remove(TEXTSTRING *text);
