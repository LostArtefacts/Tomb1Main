#pragma once

#include "global/types.h"

#include <libtrx/game/camera/vars.h>
#include <libtrx/gfx/context.h>

#include <SDL2/SDL.h>

extern GAME_FLOW_COMMAND g_GF_OverrideCommand;

extern int16_t g_RoomsToDraw[MAX_ROOMS_TO_DRAW];
extern int16_t g_RoomsToDrawCount;

extern const float g_RhwFactor;
extern uint16_t *g_TexturePageBuffer16[MAX_TEXTURE_PAGES];
extern PHD_TEXTURE g_TextureInfo[MAX_TEXTURES];

extern SDL_Window *g_SDLWindow;

extern uint32_t g_PerspectiveDistance;
extern int32_t g_CineTrackID;
extern uint32_t g_AssaultBestTime;
extern int16_t g_CineTargetAngle;
extern int32_t g_OverlayStatus;
extern bool g_GymInvOpenEnabled;
extern int32_t g_MidSort;
extern GOURAUD_ENTRY g_GouraudTable[256];
extern int32_t g_PhdWinTop;
extern PHD_SPRITE g_PhdSprites[];
extern int32_t g_LsAdder;
extern float g_FltWinBottom;
extern float g_FltResZBuf;
extern float g_FltResZ;
extern int32_t g_PhdWinHeight;
extern int32_t g_PhdWinCenterX;
extern int32_t g_PhdWinCenterY;
extern float g_FltWinTop;
extern SORT_ITEM g_SortBuffer[];
extern float g_FltWinLeft;
extern int32_t g_PhdFarZ;
extern float g_FltRhwOPersp;
extern int32_t g_PhdWinBottom;
extern int32_t g_PhdPersp;
extern int32_t g_PhdWinLeft;
extern int16_t g_Info3DBuffer[];
extern int32_t g_PhdWinMaxX;
extern int32_t g_PhdNearZ;
extern float g_FltResZORhw;
extern float g_FltFarZ;
extern float g_FltWinCenterX;
extern float g_FltWinCenterY;
extern float g_FltPerspONearZ;
extern float g_FltRhwONearZ;
extern int32_t g_PhdWinMaxY;
extern float g_FltNearZ;
extern float g_FltPersp;
extern int16_t *g_Info3DPtr;
extern int32_t g_PhdWinWidth;
extern int32_t g_PhdViewDistance;
extern DEPTHQ_ENTRY g_DepthQTable[32];
extern int32_t g_LsDivider;
extern PHD_VBUF g_PhdVBuf[];
extern uint8_t *g_TexturePageBuffer8[MAX_TEXTURE_PAGES];
extern float g_FltWinRight;
extern XYZ_32 g_LsVectorView;
extern int32_t g_PhdWinRight;
extern int32_t g_SurfaceCount;
extern SORT_ITEM *g_Sort3DPtr;
extern int32_t g_WibbleOffset;
extern bool g_IsWibbleEffect;
extern bool g_IsWaterEffect;
extern bool g_IsShadeEffect;
extern int32_t g_IsChunkyCamera;
extern int32_t g_FlipTimer;
extern bool g_IsDemoLoaded;
extern bool g_IsAssaultTimerDisplay;
extern bool g_IsAssaultTimerActive;
extern bool g_IsMonkAngry;
extern uint16_t g_SoundOptionLine;
extern ASSAULT_STATS g_Assault;
extern int32_t g_LevelItemCount;
extern int32_t g_HealthBarTimer;
extern uint8_t g_IsGameToExit;
extern int32_t g_CurrentLevel;
extern int32_t g_LevelComplete;
extern RGB_888 g_GamePalette8[256];
extern RGB_888 g_GamePalette16[256];
extern SAVEGAME_INFO g_SaveGame;
extern LARA_INFO g_Lara;
extern ITEM *g_LaraItem;
extern int16_t g_NextItemActive;
extern int16_t g_PrevItemActive;
extern int16_t g_LegacyLevelCount;
extern int32_t g_SoundEffectCount;
extern OBJECT g_Objects[265];
extern int32_t g_RoomCount;
extern ROOM *g_Rooms;
extern int32_t g_FlipStatus;
extern ITEM *g_Items;
extern int16_t g_NumCineFrames;
extern CINE_FRAME *g_CineData;
extern PHD_3DPOS g_CinePos;
extern int16_t g_CineFrameIdx;
extern BOX_INFO *g_Boxes;
extern int16_t *g_FlyZone[2];
extern int16_t *g_GroundZone[][2];
extern uint16_t *g_Overlap;
extern CREATURE *g_BaddieSlots;
extern int32_t g_DynamicLightCount;
extern STATIC_OBJECT_3D g_StaticObjects3D[MAX_STATIC_OBJECTS];
extern STATIC_OBJECT_2D g_StaticObjects2D[MAX_STATIC_OBJECTS];
extern OBJECT_VECTOR *g_SoundEffects;
extern int16_t g_SampleLUT[];
extern SAMPLE_INFO *g_SampleInfos;
extern int32_t g_HeightType;
extern int32_t g_FlipMaps[MAX_FLIP_MAPS];
extern bool g_CameraUnderwater;
extern int32_t g_BoxCount;
extern int32_t g_TexturePageCount;
extern int32_t g_TextureInfoCount;
extern uint8_t g_LabTextureUVFlag[MAX_TEXTURES];
extern int32_t g_NumCameras;
extern int16_t *g_AnimTextureRanges;
extern uint32_t *g_DemoPtr;
extern int32_t g_DemoCount;
extern char g_LevelFileName[256];
extern uint16_t g_MusicTrackFlags[64];
extern WEAPON_INFO g_Weapons[];
extern int16_t g_FinalBossActive;
extern int16_t g_FinalLevelCount;
extern int16_t g_FinalBossCount;
extern int16_t g_FinalBossItem[5];
extern REQUEST_INFO g_LoadGameRequester;
extern REQUEST_INFO g_SaveGameRequester;

extern char **g_GF_CutsceneFileNames;
extern char **g_GF_LevelFileNames;
extern char **g_GF_LevelNames;
extern char **g_GF_TitleFileNames;
extern char *g_GF_CutsceneFileNamesBuf;
extern char *g_GF_LevelFileNamesBuf;
extern char *g_GF_TitleFileNamesBuf;

extern bool g_GF_DeadlyWater;
extern bool g_GF_SunsetEnabled;
extern bool g_GF_RemoveAmmo;
extern bool g_GF_RemoveWeapons;
extern int16_t g_GF_MusicTracks[16];
extern int16_t g_GF_NoFloor;
extern int16_t g_GF_NumSecrets;
extern int32_t g_GF_LaraStartAnim;

extern int32_t g_SavedGames;
extern TEXTSTRING *g_PasswordText1;
extern char g_ValidLevelStrings1[MAX_LEVELS][50];
extern char g_ValidLevelStrings2[MAX_LEVELS][50];
extern uint32_t g_RequesterFlags1[MAX_REQUESTER_ITEMS];
extern uint32_t g_RequesterFlags2[MAX_REQUESTER_ITEMS];
extern int32_t g_SaveCounter;
extern int16_t g_SavedLevels[MAX_LEVELS];
extern XYZ_32 g_InteractPosition;
extern bool g_DetonateAllMines;
extern int32_t g_SunsetTimer;
extern NAMED_COLOR g_NamedColors[];
