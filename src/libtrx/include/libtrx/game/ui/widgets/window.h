#pragma once

#include "./base.h"

UI_WIDGET *UI_Window_Create(
    UI_WIDGET *root, int32_t border_top, int32_t border_right,
    int32_t border_bottom, int32_t border_left);

void UI_Window_SetTitle(UI_WIDGET *widget, const char *text);
void UI_Window_SetRootWidget(UI_WIDGET *widget, UI_WIDGET *root);
