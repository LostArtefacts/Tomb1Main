#pragma once

#include "global/types.h"

void __cdecl Shell_Main(void);
void __cdecl Shell_Shutdown(void);
void __cdecl Shell_ExitSystem(const char *message);
void __cdecl Shell_ExitSystemFmt(const char *fmt, ...);
