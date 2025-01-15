#pragma once

#include <stdint.h>

void S_Shell_Init(void);
void S_Shell_CreateWindow(void);
bool S_Shell_GetCommandLine(int *arg_count, char ***args);
void S_Shell_ToggleFullscreen(void);
void S_Shell_HandleWindowResize(void);
