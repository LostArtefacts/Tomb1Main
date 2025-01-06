#pragma once

extern bool Output_MakeScreenshot(const char *path);
extern void Output_BeginScene(void);
extern void Output_EndScene(void);

void Output_LoadBackgroundFromFile(const char *file_name);
void Output_UnloadBackground(void);

void Output_DrawBlackRectangle(int32_t opacity);
void Output_DrawBackground(void);
void Output_DrawPolyList(void);

extern void Output_RotateLight(int16_t pitch, int16_t yaw);
extern void Output_SetLightAdder(int32_t adder);
extern void Output_SetLightDivider(int32_t divider);
