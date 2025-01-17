#include "game/game_buf.h"

#include "enum_map.h"
#include "game/shell.h"
#include "log.h"
#include "memory.h"

static size_t m_Cap = 0;
static size_t m_MemUsed = 0;
static size_t m_MemFree = 0;
static char *m_MemBase = NULL;
static char *m_MemPtr = NULL;

void GameBuf_Init(const size_t cap)
{
    m_Cap = cap;
    m_MemBase = Memory_Alloc(cap);
    GameBuf_Reset();
}

void GameBuf_Reset(void)
{
    m_MemPtr = m_MemBase;
    m_MemFree = m_Cap;
    m_MemUsed = 0;
}

void GameBuf_Shutdown(void)
{
    Memory_FreePointer(&m_MemBase);
    m_Cap = 0;
    m_MemFree = 0;
    m_MemUsed = 0;
}

void *GameBuf_Alloc(const size_t alloc_size, const GAME_BUFFER buffer)
{
    const size_t aligned_size = (alloc_size + 3) & ~3;

    const char *const buffer_name = ENUM_MAP_TO_STRING(GAME_BUFFER, buffer);
    if (aligned_size > m_MemFree) {
        Shell_ExitSystemFmt(
            "Ran out of memory while trying to allocate %d bytes for %s",
            aligned_size, buffer_name);
    }

    void *const result = m_MemPtr;
    m_MemFree -= aligned_size;
    m_MemUsed += aligned_size;
    m_MemPtr += aligned_size;
    return result;
}
