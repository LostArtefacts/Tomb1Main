#pragma once

#include "./output/const.h"
#include "./output/types.h"
#include "./output/vars.h"
#include "./rooms.h"

#include <stdint.h>

typedef enum {
    BK_TRANSPARENT,
    BK_OBJECT,
    BK_IMAGE,
} BACKGROUND_TYPE;

extern bool Output_MakeScreenshot(const char *path);
extern void Output_BeginScene(void);
extern void Output_EndScene(void);

extern bool Output_LoadBackgroundFromFile(const char *file_name);
extern void Output_LoadBackgroundFromObject(void);
extern void Output_UnloadBackground(void);

extern void Output_DrawBlackRectangle(int32_t opacity);
extern void Output_DrawBackground(void);
extern void Output_DrawPolyList(void);

extern void Output_SetupBelowWater(bool is_underwater);
extern void Output_SetupAboveWater(bool is_underwater);
extern void Output_RotateLight(int16_t pitch, int16_t yaw);
extern void Output_SetLightAdder(int32_t adder);
extern void Output_SetLightDivider(int32_t divider);

// Temporary
extern int32_t Output_CalcFogShade(int32_t depth);
extern int32_t Output_GetRoomLightShade(ROOM_LIGHT_MODE mode);
extern void Output_LightRoomVertices(const ROOM *room);

void Output_InitialiseAnimatedTextures(int32_t num_ranges);
void Output_CycleAnimatedTextures(void);

void Output_CalculateLight(XYZ_32 pos, int16_t room_num);
void Output_CalculateStaticLight(int16_t adder);
void Output_CalculateStaticMeshLight(XYZ_32 pos, SHADE shade, const ROOM *room);
void Output_CalculateObjectLighting(const ITEM *item, const BOUNDS_16 *bounds);
void Output_LightRoom(ROOM *room);

void Output_ResetDynamicLights(void);
void Output_AddDynamicLight(XYZ_32 pos, int32_t intensity, int32_t falloff);
