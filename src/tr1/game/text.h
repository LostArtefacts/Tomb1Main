#pragma once

#include "config.h"

#include <libtrx/game/text.h>

#include <stdbool.h>
#include <stdint.h>

#define TEXT_HEIGHT 11 // TODO: Get rid of this
#define TEXT_OUTLINE_THICKNESS 2

RGBA_8888 Text_GetMenuColor(MENU_COLOR color);
void Text_DrawText(TEXTSTRING *text);
