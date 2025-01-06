#pragma once

#include "global/types.h"

#include <libtrx/game/overlay.h>

void Overlay_DrawAssaultTimer(void);
void Overlay_DrawGameInfo(bool pickup_state);
void Overlay_DrawHealthBar(void);
void Overlay_DrawAirBar(void);
void Overlay_HideGameInfo(void);
void Overlay_DrawAmmoInfo(void);
void Overlay_InitialisePickUpDisplay(void);
void Overlay_DrawPickups(bool pickup_state);
void Overlay_AddDisplayPickup(int16_t object_id);
void Overlay_DisplayModeInfo(const char *string);

void Overlay_DrawModeInfo(void);
void Overlay_Animate(int32_t ticks);
