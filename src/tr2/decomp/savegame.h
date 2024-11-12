#pragma once

#include "global/types.h"

void __cdecl InitialiseStartInfo(void);
void __cdecl ModifyStartInfo(int32_t level_num);
void __cdecl CreateStartInfo(int32_t level_num);
void __cdecl CreateSaveGameInfo(void);
void __cdecl ExtractSaveGameInfo(void);

void __cdecl ResetSG(void);
void __cdecl WriteSG(const void *pointer, size_t size);
void __cdecl ReadSG(void *pointer, size_t size);

void __cdecl GetSavedGamesList(REQUEST_INFO *req);
