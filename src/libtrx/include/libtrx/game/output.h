#pragma once

#include "./output/const.h"
#include "./output/types.h"
#include "./output/vars.h"

#include <stdint.h>

typedef enum {
    BK_TRANSPARENT,
    BK_OBJECT,
    BK_IMAGE,
} BACKGROUND_TYPE;

extern bool Output_MakeScreenshot(const char *path);
extern void Output_BeginScene(void);
extern void Output_EndScene(void);

extern void Output_LoadBackgroundFromFile(const char *file_name);
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
