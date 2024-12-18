#pragma once

#include "global/types.h"

#include <stdint.h>
#include <windows.h>

// TODO: This is a placeholder file containing functions that originated from
// the decompilation phase, and they are currently disorganized. Eventually,
// they'll need to be properly modularized. The same applies to all files
// within the decomp/ directory which are scheduled for extensive refactoring.

int32_t __stdcall WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
    int32_t nShowCmd);
int16_t __cdecl TitleSequence(void);
HWND __cdecl WinVidFindGameWindow(void);
void __cdecl Game_SetCutsceneTrack(int32_t track);
void __cdecl CutscenePlayer_Control(int16_t item_num);
void __cdecl Lara_Control_Cutscene(int16_t item_num);
void __cdecl CutscenePlayer1_Initialise(int16_t item_num);
void __cdecl CutscenePlayerGen_Initialise(int16_t item_num);
int32_t __cdecl Level_Initialise(
    int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
int32_t __cdecl Misc_Move3DPosTo3DPos(
    PHD_3DPOS *src_pos, const PHD_3DPOS *dst_pos, int32_t velocity,
    PHD_ANGLE ang_add);
int32_t __cdecl LevelCompleteSequence(void);
void __cdecl S_Wait(int32_t frames, BOOL input_check);
BOOL __cdecl S_InitialiseSystem(void);
void __cdecl DisplayCredits(void);
void __cdecl S_InitialisePolyList(bool clear_back_buffer);
void __cdecl DecreaseScreenSize(void);
void __cdecl IncreaseScreenSize(void);
BOOL __cdecl S_LoadLevelFile(
    const char *file_name, int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
void __cdecl S_UnloadLevelFile(void);
void __cdecl GetValidLevelsList(REQUEST_INFO *req);
void __cdecl InitialiseGameFlags(void);
void __cdecl InitialiseLevelFlags(void);
void __cdecl GetCarriedItems(void);
int32_t __cdecl DoShift(ITEM *vehicle, const XYZ_32 *pos, const XYZ_32 *old);
int32_t __cdecl DoDynamics(int32_t height, int32_t fall_speed, int32_t *out_y);
