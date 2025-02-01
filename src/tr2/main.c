#include "decomp/decomp.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

int main(int argc, char **argv)
{
    char *log_path = File_GetFullPath("TR2X.log");
    Log_Init(log_path);
    Memory_Free(log_path);

    Shell_Setup();
    Shell_Main();
    Shell_Terminate(0);
    return 0;
}
