#pragma once

#include "global/types.h"

#include <libtrx/virtual_file.h>

void __cdecl Level_LoadPalettes(VFILE *file);
void __cdecl Level_LoadTexturePages(VFILE *file);
void __cdecl Level_LoadDepthQ(VFILE *file);

bool __cdecl Level_Load(const char *file_name, int32_t level_num);
