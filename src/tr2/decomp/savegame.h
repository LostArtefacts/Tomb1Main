#pragma once

#include "game/game_flow/types.h"
#include "global/types.h"

#include <stddef.h>

void InitialiseStartInfo(void);
void ModifyStartInfo(const GAME_FLOW_LEVEL *level);
void CreateStartInfo(const GAME_FLOW_LEVEL *level);
void CreateSaveGameInfo(void);
void ExtractSaveGameInfo(void);

void ResetSG(void);
void WriteSG(const void *pointer, size_t size);
void ReadSG(void *pointer, size_t size);

void GetSavedGamesList(REQUEST_INFO *req);
bool S_FrontEndCheck(void);
int32_t S_SaveGame(int32_t slot_num);
int32_t S_LoadGame(int32_t slot_num);
