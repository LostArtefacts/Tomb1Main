#include "global/vars.h"

#include <stddef.h>

int32_t g_FPSCounter = 0;

int16_t g_SampleLUT[MAX_SAMPLES] = {};
SAMPLE_INFO *g_SampleInfos = NULL;
uint16_t g_MusicTrackFlags[MAX_CD_TRACKS] = {};

bool g_IDelay = false;
int32_t g_IDCount = 0;
int32_t g_OptionSelected = 0;

int32_t g_PhdPersp = 0;
int32_t g_PhdLeft = 0;
int32_t g_PhdBottom = 0;
int32_t g_PhdRight = 0;
int32_t g_PhdTop = 0;
float g_FltResZ;
float g_FltResZBuf;

PHD_SPRITE g_PhdSpriteInfo[MAX_SPRITES] = {};
PHD_TEXTURE g_PhdTextureInfo[MAX_TEXTURES] = {};

LARA_INFO g_Lara = {};
ITEM *g_LaraItem = NULL;
GAME_INFO g_GameInfo = { .override_gf_command = { .action = GF_NOOP }, 0 };
int32_t g_SavedGamesCount = 0;
int32_t g_SaveCounter = 0;
int16_t g_CurrentLevel = -1;
uint32_t *g_DemoData = NULL;
bool g_LevelComplete = false;
bool g_ChunkyFlag = false;
int32_t g_OverlayFlag = 0;
int32_t g_HeightType = 0;

ROOM *g_RoomInfo = NULL;
OBJECT g_Objects[O_NUMBER_OF] = {};
STATIC_INFO g_StaticObjects[STATIC_NUMBER_OF] = {};
RGBA_8888 *g_TexturePagePtrs[MAX_TEXTPAGES] = { NULL };
int16_t g_RoomCount = 0;
int32_t g_LevelItemCount = 0;
int32_t g_NumberBoxes = 0;
BOX_INFO *g_Boxes = NULL;
uint16_t *g_Overlap = NULL;
int16_t *g_GroundZone[2] = { NULL };
int16_t *g_GroundZone2[2] = { NULL };
int16_t *g_FlyZone[2] = { NULL };
TEXTURE_RANGE *g_AnimTextureRanges = NULL;
int16_t g_NumCineFrames = 0;
int16_t g_CineFrame = -1;
CINE_CAMERA *g_CineCamera = NULL;
CINE_POSITION g_CinePosition = {};
int32_t g_NumberCameras = 0;
int32_t g_NumberSoundEffects = 0;
OBJECT_VECTOR *g_SoundEffectsTable = NULL;
int16_t g_RoomsToDraw[MAX_ROOMS_TO_DRAW] = { -1 };
int16_t g_RoomsToDrawCount = 0;

INVENTORY_MODE g_InvMode;

#ifndef MESON_BUILD
const char *g_TRXVersion = "TR1X (non-Docker build)";
#endif
