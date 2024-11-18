#pragma once

#include <libtrx/game/gamebuf.h>

#include <stdint.h>

void GameBuf_Init(int32_t cap);
void __cdecl GameBuf_Reset(void);
void __cdecl GameBuf_Shutdown(void);
void __cdecl GameBuf_Free(size_t free_size);
