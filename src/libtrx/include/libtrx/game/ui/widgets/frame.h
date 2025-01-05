#pragma once

#include "./base.h"

UI_WIDGET *UI_Frame_Create(UI_WIDGET *root, int32_t margin, int32_t padding);
void UI_Frame_SetFrameVisible(UI_WIDGET *widget, bool is_frame_visible);
