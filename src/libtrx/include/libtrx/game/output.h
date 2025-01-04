#pragma once

extern bool Output_MakeScreenshot(const char *path);
extern void Output_BeginScene(void);
extern void Output_EndScene(void);

void Output_LoadBackgroundFromFile(const char *file_name);
void Output_UnloadBackground(void);

void Output_DrawBlackRectangle(int32_t opacity);
void Output_DrawBackground(void);
void Output_DrawPolyList(void);
