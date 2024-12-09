#pragma once

#include <libtrx/game/shell.h>

#include <stdbool.h>

void Shell_Init(const char *gameflow_path);
const char *Shell_GetGameflowPath(void);
void Shell_ProcessInput(void);
void Shell_Main(void);
