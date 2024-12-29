#include "decomp/decomp.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>
#include <libtrx/log.h>

#include <windows.h>

int32_t __stdcall WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
    int32_t nShowCmd)
{
    Log_Init(File_GetFullPath("TR2X.log"));
    g_IsGameToExit = false;
    Shell_Setup();
    Shell_Main();
    Shell_Shutdown();

cleanup:
    return 0;
}
