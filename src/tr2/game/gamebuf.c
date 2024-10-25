#include "game/gamebuf.h"

#include "global/vars.h"

#include <stddef.h>
#include <windows.h>

void __cdecl GameBuf_Shutdown(void)
{
    if (g_GameBuf_MemBase != NULL) {
        GlobalFree(g_GameBuf_MemBase);
    }
}
