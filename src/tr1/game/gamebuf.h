#pragma once

#include <libtrx/game/gamebuf.h>

#include <stdint.h>

// Internal game memory manager. It allocates its internal buffer once per
// level launch. All subsequent "allocation" requests operate with pointer
// arithmetic. This makes it fast and convenient to request more memory as we
// go, but it makes freeing memory really inconvenient which is why it is
// intentionally not implemented. To use more dynamic memory management, use
// Memory_Alloc / Memory_Free.

void GameBuf_Init(void);
void GameBuf_Reset(void);
void GameBuf_Shutdown(void);
