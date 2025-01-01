#include "decomp/decomp.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>
#include <libtrx/log.h>

int main(int argc, char **argv)
{
    Log_Init(File_GetFullPath("TR2X.log"));
    g_IsGameToExit = false;
    Shell_Setup();
    Shell_Main();
    Shell_Shutdown();
    return 0;
}
