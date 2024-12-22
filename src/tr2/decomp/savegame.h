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
bool __cdecl S_FrontEndCheck(void);
int32_t __cdecl S_SaveGame(
    const void *save_data, size_t save_size, int32_t slot_num);
int32_t __cdecl S_LoadGame(void *save_data, size_t save_size, int32_t slot_num);
