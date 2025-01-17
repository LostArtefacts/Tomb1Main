#pragma once

#include "global/types.h"

#include <libtrx/game/overlay.h>

void Overlay_Reset(void);
void Overlay_HideGameInfo(void);
void Overlay_AddDisplayPickup(GAME_OBJECT_ID object_id);
void Overlay_DisplayModeInfo(const char *string);

void Overlay_DrawGameInfo(void);
void Overlay_DrawHealthBar(void);

void Overlay_Animate(int32_t frames);
