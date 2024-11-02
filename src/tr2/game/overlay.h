#pragma once

#include "global/types.h"

void __cdecl Overlay_DrawAssaultTimer(void);
void __cdecl Overlay_DrawGameInfo(bool pickup_state);
void Overlay_DrawHealthBar(void);
void Overlay_DrawAirBar(void);
void Overlay_HideGameInfo(void);
void __cdecl Overlay_MakeAmmoString(char *string);
void __cdecl Overlay_DrawAmmoInfo(void);
void __cdecl Overlay_InitialisePickUpDisplay(void);
void __cdecl Overlay_DrawPickups(bool pickup_state);
void __cdecl Overlay_AddDisplayPickup(int16_t object_id);
void __cdecl Overlay_DisplayModeInfo(const char *string);

void __cdecl Overlay_DrawModeInfo(void);
void Overlay_Animate(int32_t ticks);
