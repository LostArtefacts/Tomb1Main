#pragma once

#include "global/const.h"
#include "global/types.h"

#include <libtrx/game/camera/vars.h>
#include <libtrx/game/inventory_ring/enum.h>

#include <stddef.h>
#include <stdint.h>

extern int32_t g_PhdPersp;
extern int32_t g_PhdLeft;
extern int32_t g_PhdBottom;
extern int32_t g_PhdRight;
extern int32_t g_PhdTop;
extern float g_FltResZ;
extern float g_FltResZBuf;

extern int32_t g_OptionSelected;

extern int16_t g_SampleLUT[MAX_SAMPLES];
extern SAMPLE_INFO *g_SampleInfos;
extern uint16_t g_MusicTrackFlags[MAX_CD_TRACKS];
extern int32_t g_Sound_MasterVolume;

extern int32_t g_FPSCounter;

extern LARA_INFO g_Lara;
extern ITEM *g_LaraItem;
extern GAME_INFO g_GameInfo;
extern int32_t g_SavedGamesCount;
extern int32_t g_SaveCounter;
extern uint32_t *g_DemoData;
extern bool g_LevelComplete;
extern bool g_ChunkyFlag;
extern int32_t g_OverlayFlag;
extern int32_t g_HeightType;

extern ROOM *g_RoomInfo;
extern OBJECT g_Objects[O_NUMBER_OF];
extern STATIC_OBJECT_3D g_StaticObjects3D[MAX_STATIC_OBJECTS];
extern STATIC_OBJECT_2D g_StaticObjects2D[MAX_STATIC_OBJECTS];
extern RGBA_8888 *g_TexturePagePtrs[MAX_TEXTURE_PAGES];
extern int16_t g_RoomCount;
extern int32_t g_LevelItemCount;
extern int32_t g_NumberBoxes;
extern BOX_INFO *g_Boxes;
extern uint16_t *g_Overlap;
extern int16_t *g_GroundZone[2];
extern int16_t *g_GroundZone2[2];
extern int16_t *g_FlyZone[2];
extern int16_t g_NumCineFrames;
extern int16_t g_CineFrame;
extern CINE_CAMERA *g_CineCamera;
extern CINE_POSITION g_CinePosition;
extern int32_t g_NumberCameras;
extern int32_t g_NumberSoundEffects;
extern OBJECT_VECTOR *g_SoundEffectsTable;
extern int16_t g_RoomsToDraw[MAX_ROOMS_TO_DRAW];
extern int16_t g_RoomsToDrawCount;

extern REQUEST_INFO g_SavegameRequester;

extern INVENTORY_MODE g_InvMode;
