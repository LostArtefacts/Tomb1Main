#pragma once

#include <stdbool.h>
#include <stdint.h>

void S_Shell_Init(void);
void S_Shell_CreateWindow(void);
void S_Shell_SpinMessageLoop(void);
bool S_Shell_GetCommandLine(int *arg_count, char ***args);
void S_Shell_ToggleFullscreen(void);
void S_Shell_HandleWindowResize(void);
