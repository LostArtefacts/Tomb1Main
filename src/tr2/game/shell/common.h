#pragma once

#include "global/types.h"

#include <libtrx/game/shell.h>

void Shell_Main(void);
void Shell_Start();

void Shell_ProcessEvents(void);

bool Shell_IsFullscreen(void);
