#include "global/vars.h"

int32_t g_FPSCounter = 0;

int16_t g_SampleLUT[MAX_SAMPLES] = {};
SAMPLE_INFO *g_SampleInfos = nullptr;
uint16_t g_MusicTrackFlags[MAX_CD_TRACKS] = {};

int32_t g_OptionSelected = 0;

int32_t g_PhdPersp = 0;
int32_t g_PhdLeft = 0;
int32_t g_PhdBottom = 0;
int32_t g_PhdRight = 0;
int32_t g_PhdTop = 0;
float g_FltResZ;
float g_FltResZBuf;

LARA_INFO g_Lara = {};
ITEM *g_LaraItem = nullptr;
GAME_INFO g_GameInfo = {};
int32_t g_SavedGamesCount = 0;
int32_t g_SaveCounter = 0;
uint32_t *g_DemoData = nullptr;
bool g_LevelComplete = false;
int32_t g_OverlayFlag = 0;
int32_t g_HeightType = 0;

ROOM *g_RoomInfo = nullptr;
OBJECT g_Objects[O_NUMBER_OF] = {};
STATIC_OBJECT_3D g_StaticObjects3D[MAX_STATIC_OBJECTS] = {};
STATIC_OBJECT_2D g_StaticObjects2D[MAX_STATIC_OBJECTS] = {};
int16_t g_RoomCount = 0;
int32_t g_LevelItemCount = 0;
int32_t g_NumberBoxes = 0;
BOX_INFO *g_Boxes = nullptr;
uint16_t *g_Overlap = nullptr;
int16_t *g_GroundZone[2] = { nullptr };
int16_t *g_GroundZone2[2] = { nullptr };
int16_t *g_FlyZone[2] = { nullptr };
int32_t g_NumberSoundEffects = 0;
OBJECT_VECTOR *g_SoundEffectsTable = nullptr;
int16_t g_RoomsToDraw[MAX_ROOMS_TO_DRAW] = { -1 };
int16_t g_RoomsToDrawCount = 0;

INVENTORY_MODE g_InvMode;

#ifndef MESON_BUILD
const char *g_TRXVersion = "TR1X (non-Docker build)";
#endif
