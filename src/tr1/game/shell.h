#pragma once

#include <stdbool.h>

void Shell_Init(const char *gameflow_path);
const char *Shell_GetGameflowPath(void);
void Shell_ProcessInput(void);
void Shell_Shutdown(void);
void Shell_Main(void);
void Shell_ExitSystem(const char *message);
void Shell_ExitSystemFmt(const char *fmt, ...);
