#include "game/gamebuf.h"

#include "global/vars.h"

#include <libtrx/memory.h>

#include <stddef.h>

static int32_t m_Cap = 0;

void GameBuf_Init(const int32_t cap)
{
    m_Cap = cap;
    g_GameBuf_MemBase = Memory_Alloc(cap);
}

void __cdecl GameBuf_Reset(void)
{
    g_GameBuf_MemPtr = g_GameBuf_MemBase;
    g_GameBuf_MemFree = m_Cap;
    g_GameBuf_MemUsed = 0;
}

void __cdecl GameBuf_Shutdown(void)
{
    Memory_FreePointer(&g_GameBuf_MemBase);
    m_Cap = 0;
    g_GameBuf_MemFree = 0;
    g_GameBuf_MemUsed = 0;
}
