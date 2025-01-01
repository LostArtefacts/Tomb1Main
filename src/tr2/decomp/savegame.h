#pragma once

#include "global/types.h"

#include <stddef.h>

void InitialiseStartInfo(void);
void ModifyStartInfo(int32_t level_num);
void CreateStartInfo(int32_t level_num);
void CreateSaveGameInfo(void);
void ExtractSaveGameInfo(void);

void ResetSG(void);
void WriteSG(const void *pointer, size_t size);
void ReadSG(void *pointer, size_t size);

void GetSavedGamesList(REQUEST_INFO *req);
bool S_FrontEndCheck(void);
int32_t S_SaveGame(const void *save_data, size_t save_size, int32_t slot_num);
int32_t S_LoadGame(void *save_data, size_t save_size, int32_t slot_num);
