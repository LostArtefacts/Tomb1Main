#include "global/vars.h"

#include "game/spawn.h"

#include <libtrx/game/sound/ids.h>

#ifndef MESON_BUILD
const char *g_TR2XVersion = "TR2X (non-Docker build)";
#endif

GAME_FLOW_DIR g_GF_OverrideDir = (GAME_FLOW_DIR)-1;

int16_t g_RoomsToDraw[MAX_ROOMS_TO_DRAW] = { 0 };
int16_t g_RoomsToDrawCount = 0;

void *g_XBuffer = NULL;
const float g_RhwFactor = 0x14000000.p0;
uint16_t *g_TexturePageBuffer16[MAX_TEXTURE_PAGES] = { 0 };
PHD_TEXTURE g_TextureInfo[MAX_TEXTURES];

SDL_Window *g_SDLWindow = NULL;

uint32_t g_PerspectiveDistance = 0x3000000;
int32_t g_CineTrackID = 1;
int32_t g_CineTickRate = 0x8000; // PHD_ONE/TICKS_PER_FRAME
uint32_t g_AssaultBestTime = -1;

int16_t g_CineTargetAngle = PHD_90;
int32_t g_OverlayStatus = 1;
int16_t g_Inv_MainObjectsCount = 8;
BOOL g_GymInvOpenEnabled = TRUE;
int16_t g_Inv_Chosen = -1;
INVENTORY_MODE g_Inv_Mode = INV_TITLE_MODE;
int32_t g_JumpPermitted = 1;
int16_t g_LaraOldSlideAngle = 1;
int32_t g_FadeValue = 0x100000;
int32_t g_FadeLimit = 0x100000;
int32_t g_FadeAdder = 0x8000;
int32_t g_MidSort = 0;
float g_ViewportAspectRatio = 0.0f;
int32_t g_XGenY1 = 0;
int32_t g_XGenY2 = 0;
GOURAUD_ENTRY g_GouraudTable[256];
int32_t g_PhdWinTop;
PHD_SPRITE g_PhdSprites[512];
int32_t g_LsAdder;
float g_FltWinBottom;
float g_FltResZBuf;
float g_FltResZ;
int32_t g_PhdWinHeight;
int32_t g_PhdWinCenterX;
int32_t g_PhdWinCenterY;
int16_t g_LsYaw;
float g_FltWinTop;
SORT_ITEM g_SortBuffer[4000];
float g_FltWinLeft;
int32_t g_PhdFarZ;
float g_FltRhwOPersp;
int32_t g_PhdWinBottom;
int32_t g_PhdPersp;
int32_t g_PhdWinLeft;
int16_t g_Info3DBuffer[120000];
int32_t g_PhdWinMaxX;
int32_t g_PhdNearZ;
float g_FltResZORhw;
float g_FltFarZ;
float g_FltWinCenterX;
float g_FltWinCenterY;
float g_FltPerspONearZ;
float g_FltRhwONearZ;
int32_t g_PhdWinMaxY;
float g_FltNearZ;
int32_t g_RandomTable[32];
float g_FltPersp;
int16_t *g_Info3DPtr = NULL;
int32_t g_PhdWinWidth;
int32_t g_PhdViewDistance;
int16_t g_LsPitch;
int16_t g_ShadesTable[32];
DEPTHQ_ENTRY g_DepthQTable[32];
int32_t g_LsDivider;
PHD_VBUF g_PhdVBuf[1500];
uint8_t *g_TexturePageBuffer8[MAX_TEXTURE_PAGES] = {};
float g_FltWinRight;
XYZ_32 g_LsVectorView;
float g_WibbleTable[32];
int32_t g_PhdWinRight;
int32_t g_SurfaceCount;
SORT_ITEM *g_Sort3DPtr = NULL;
int32_t g_WibbleOffset;
int32_t g_IsWibbleEffect;
int32_t g_IsWaterEffect;
int8_t g_IsShadeEffect;
PALETTEENTRY g_GamePalette16[256];
int32_t g_IsChunkyCamera;
int32_t g_NoInputCounter;
int32_t g_FlipTimer;
int32_t g_LOSNumRooms = 0;
BOOL g_IsDemoLevelType;
BOOL g_IsDemoLoaded;
int32_t g_BoundStart;
int32_t g_BoundEnd;
int32_t g_IsAssaultTimerDisplay;
BOOL g_IsAssaultTimerActive;
BOOL g_IsMonkAngry;
TEXTSTRING *g_AmmoTextInfo = NULL;
TEXTSTRING *g_DisplayModeTextInfo = NULL;
DWORD g_DisplayModeInfoTimer;
UINT16 g_Inv_MainCurrent;
UINT16 g_Inv_KeyObjectsCount;
UINT16 g_Inv_KeysCurrent;
UINT16 g_Inv_OptionCurrent;
TEXTSTRING *g_Inv_RingText = NULL;
TEXTSTRING *g_Inv_UpArrow1 = NULL;
TEXTSTRING *g_Inv_UpArrow2 = NULL;
TEXTSTRING *g_Inv_DownArrow1 = NULL;
TEXTSTRING *g_Inv_DownArrow2 = NULL;
uint16_t g_Inv_IsActive;
BOOL g_Inv_DemoMode;
BOOL g_Inv_IsOptionsDelay;
int32_t g_Inv_OptionsDelayCounter;
uint16_t g_SoundOptionLine;
REQUEST_INFO g_StatsRequester;
ASSAULT_STATS g_Assault;
int32_t g_LevelItemCount;
int32_t g_HealthBarTimer;
int32_t g_SoundTrackIds[128];
int32_t g_MinWindowClientWidth;
int32_t g_MinWindowClientHeight;
int32_t g_MinWindowWidth;
int32_t g_MinWindowHeight;
bool g_IsMinWindowSizeSet;
int32_t g_MaxWindowClientWidth;
HWND g_GameWindowHandle;
uint32_t g_LockedBufferCount;
int32_t g_MaxWindowClientHeight;
int32_t g_MaxWindowWidth;
int32_t g_MaxWindowHeight;
bool g_IsMaxWindowSizeSet;
uint32_t g_AppResultCode;
uint8_t g_IsGameToExit;
uint8_t g_IsSoundEnabled;
int32_t g_IsFMVPlaying;
int32_t g_CurrentLevel;
int32_t g_LevelComplete;
char *g_GameBuf_MemBase = NULL;
char *g_CmdLine = NULL;
RGB_888 g_GamePalette8[256];
int32_t g_CD_LoopTrack;
int32_t g_SoundIsActive;
SAVEGAME_INFO g_SaveGame;
LARA_INFO g_Lara;
ITEM *g_LaraItem = NULL;
int16_t g_NextItemFree;
int16_t g_NextItemActive;
int16_t g_PrevItemActive;
PICKUP_INFO g_Pickups[12];
GAME_FLOW g_GameFlow;
int32_t g_SoundEffectCount;
OBJECT g_Objects[265] = { 0 };
int16_t **g_Meshes = NULL;
int32_t g_IMFrac;
ANIM *g_Anims = NULL;
int32_t *g_AnimBones = NULL;
int32_t g_RoomCount;
int32_t g_IMRate;
ROOM *g_Rooms = NULL;
int32_t g_FlipStatus;
int16_t *g_Legacy_TriggerIndex = NULL;
int32_t g_LOSRooms[200];
ITEM *g_Items = NULL;
int16_t g_NumCineFrames;
CINE_FRAME *g_CineData = NULL;
PHD_3DPOS g_CinePos;
int16_t g_CineFrameIdx;
CAMERA_INFO g_Camera;
BOX_INFO *g_Boxes = NULL;
BOOL g_IsTitleLoaded;
int32_t g_Inv_ExtraData[8];
int32_t g_MessageLoopCounter;
bool g_IsGameWindowActive;
int16_t *g_FlyZone[2] = {};
int16_t *g_GroundZone[4][2] = {};
uint16_t *g_Overlap = NULL;
CREATURE *g_BaddieSlots = NULL;
int32_t g_DynamicLightCount;
int32_t g_OriginalRoom;

INVENTORY_ITEM *g_Inv_MainList[23] = {
    &g_Inv_Item_Stopwatch, &g_Inv_Item_Flare,     &g_Inv_Item_Pistols,
    &g_Inv_Item_Shotgun,   &g_Inv_Item_Magnums,   &g_Inv_Item_Uzis,
    &g_Inv_Item_M16,       &g_Inv_Item_Grenade,   &g_Inv_Item_Harpoon,
    &g_Inv_Item_LargeMedi, &g_Inv_Item_SmallMedi, NULL
};

INVENTORY_ITEM *g_Inv_KeysList[23] = {
    &g_Inv_Item_Puzzle1,
    &g_Inv_Item_Puzzle2,
    &g_Inv_Item_Puzzle3,
    &g_Inv_Item_Puzzle4,
    &g_Inv_Item_Key1,
    &g_Inv_Item_Key2,
    &g_Inv_Item_Key3,
    &g_Inv_Item_Key4,
    &g_Inv_Item_Pickup1,
    &g_Inv_Item_Pickup2,
    NULL,
};

int32_t g_Inv_NFrames = 2;
STATIC_INFO g_StaticObjects[MAX_STATIC_OBJECTS];
OBJECT_VECTOR *g_SoundEffects = NULL;
int16_t g_SampleLUT[SFX_NUMBER_OF];
SAMPLE_INFO *g_SampleInfos = NULL;
int32_t g_HeightType;
int16_t *g_Legacy_FloorData = NULL;
int16_t *g_AnimCommands = NULL;
ANIM_CHANGE *g_AnimChanges = NULL;
ANIM_RANGE *g_AnimRanges = NULL;
int32_t g_FlipMaps[MAX_FLIP_MAPS];
int32_t g_Outside;
int32_t g_OutsideRight;
int32_t g_OutsideLeft;
int32_t g_OutsideTop;
int32_t g_OutsideBottom;
int32_t g_BoundRooms[MAX_BOUND_ROOMS];
PORTAL_VBUF g_DoorVBuf[4];
int32_t g_BoxLines[12][2] = {
    { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
    { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
};
BOOL g_CameraUnderwater;
int32_t g_BoxCount;
int32_t g_SlotsUsed;
int32_t g_TexturePageCount;
int16_t *g_MeshBase = NULL;
int32_t g_TextureInfoCount;
uint8_t g_LabTextureUVFlag[MAX_TEXTURES];
FRAME_INFO *g_AnimFrames = NULL;
int32_t g_IsWet;
uint8_t g_DepthQIndex[256];
int32_t g_NumCameras;
int16_t *g_AnimTextureRanges = NULL;
int16_t g_CineLoaded;
uint32_t *g_DemoPtr = NULL;
int32_t g_DemoCount;
int32_t g_NumSampleInfos;
int32_t g_LevelFilePalettesOffset;
int32_t g_LevelFileTexPagesOffset;
char g_LevelFileName[256];
uint16_t g_MusicTrackFlags[64];

WEAPON_INFO g_Weapons[] = {
    {},
    {
        .lock_angles = { 54616, 10920, 54616, 10920 },
        .left_angles = { 34596, 10920, 50976, 14560 },
        .right_angles = { 54616, 30940, 50976, 14560 },
        .aim_speed = 1820,
        .shot_accuracy = 1456,
        .gun_height = 650,
        .damage = 1,
        .target_dist = 8192,
        .recoil_frame = 9,
        .flash_time = 3,
        .sample_num = 8,
    },
    {
        .lock_angles = { 54616, 10920, 54616, 10920 },
        .left_angles = { 34596, 10920, 50976, 14560 },
        .right_angles = { 54616, 30940, 50976, 14560 },
        .aim_speed = 1820,
        .shot_accuracy = 1456,
        .gun_height = 650,
        .damage = 2,
        .target_dist = 8192,
        .recoil_frame = 9,
        .flash_time = 3,
        .sample_num = 21,
    },
    {
        .lock_angles = { 54616, 10920, 54616, 10920 },
        .left_angles = { 34596, 10920, 50976, 14560 },
        .right_angles = { 54616, 30940, 50976, 14560 },
        .aim_speed = 1820,
        .shot_accuracy = 1456,
        .gun_height = 650,
        .damage = 1,
        .target_dist = 8192,
        .recoil_frame = 3,
        .flash_time = 3,
        .sample_num = 43,
    },
    {
        .lock_angles = { 54616, 10920, 55526, 10010 },
        .left_angles = { 50976, 14560, 53706, 11830 },
        .right_angles = { 50976, 14560, 53706, 11830 },
        .aim_speed = 1820,
        .shot_accuracy = 0,
        .gun_height = 500,
        .damage = 3,
        .target_dist = 8192,
        .recoil_frame = 9,
        .flash_time = 3,
        .sample_num = 45,
    },
    {
        .lock_angles = { 54616, 10920, 55526, 10010 },
        .left_angles = { 50976, 14560, 53706, 11830 },
        .right_angles = { 50976, 14560, 53706, 11830 },
        .aim_speed = 1820,
        .shot_accuracy = 728,
        .gun_height = 500,
        .damage = 3,
        .target_dist = 12288,
        .recoil_frame = 0,
        .flash_time = 3,
        .sample_num = 0,
    },
    {
        .lock_angles = { 54616, 10920, 55526, 10010 },
        .left_angles = { 50976, 14560, 53706, 11830 },
        .right_angles = { 50976, 14560, 53706, 11830 },
        .aim_speed = 1820,
        .shot_accuracy = 1456,
        .gun_height = 500,
        .damage = 30,
        .target_dist = 8192,
        .recoil_frame = 0,
        .flash_time = 2,
        .sample_num = 0,
    },
    {
        .lock_angles = { 54616, 10920, 53706, 11830 },
        .left_angles = { 50976, 14560, 51886, 13650 },
        .right_angles = { 50976, 14560, 51886, 13650 },
        .aim_speed = 1820,
        .shot_accuracy = 1456,
        .gun_height = 500,
        .damage = 4,
        .target_dist = 8192,
        .recoil_frame = 0,
        .flash_time = 2,
        .sample_num = 0,
    },
    {},
    {
        .lock_angles = { 60076, 5460, 55526, 10010 },
        .left_angles = { 60076, 5460, 55526, 10010 },
        .right_angles = { 60076, 5460, 55526, 10010 },
        .aim_speed = 1820,
        .shot_accuracy = 1456,
        .gun_height = 400,
        .damage = 3,
        .target_dist = 8192,
        .recoil_frame = 0,
        .flash_time = 2,
        .sample_num = 43,
    },
};

int16_t g_FinalBossActive;
int16_t g_FinalLevelCount;
int16_t g_FinalBossCount;
int16_t g_FinalBossItem[5];

INVENTORY_ITEM g_Inv_Item_Stopwatch = {
    .string = NULL,
    .object_id = O_COMPASS_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4352,
    .x_rot_pt = 0,
    .x_rot_sel = 64000,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 4294967126,
    .y_trans = 0,
    .z_trans_sel = 320,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 0,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Pistols = {
    .string = NULL,
    .object_id = O_PISTOL_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 2848,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 32768,
    .y_rot = 0,
    .y_trans_sel = 38,
    .y_trans = 0,
    .z_trans_sel = 352,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 1,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Flare = {
    .string = NULL,
    .object_id = O_FLARES_OPTION,
    .frames_total = 31,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 30,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 0,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 57344,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 8,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Shotgun = {
    .string = NULL,
    .object_id = O_SHOTGUN_OPTION,
    .frames_total = 13,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 12,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 5120,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 30720,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 228,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 2,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Magnums = {
    .string = NULL,
    .object_id = O_MAGNUM_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 3360,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 32768,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 362,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Uzis = {
    .string = NULL,
    .object_id = O_UZI_OPTION,
    .frames_total = 13,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 12,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 2336,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 32768,
    .y_rot = 0,
    .y_trans_sel = 56,
    .y_trans = 0,
    .z_trans_sel = 322,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 4,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Harpoon = {
    .string = NULL,
    .object_id = O_HARPOON_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 64800,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 46080,
    .y_rot = 0,
    .y_trans_sel = 58,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 7,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_M16 = {
    .string = NULL,
    .object_id = O_M16_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 65312,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 47104,
    .y_rot = 0,
    .y_trans_sel = 84,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Grenade = {
    .string = NULL,
    .object_id = O_GRENADE_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 65312,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 14336,
    .y_rot = 0,
    .y_trans_sel = 56,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 6,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_PistolAmmo = {
    .string = NULL,
    .object_id = O_PISTOL_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 1,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_ShotgunAmmo = {
    .string = NULL,
    .object_id = O_SHOTGUN_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 2,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_MagnumAmmo = {
    .string = NULL,
    .object_id = O_MAGNUM_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_UziAmmo = {
    .string = NULL,
    .object_id = O_UZI_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 4,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_HarpoonAmmo = {
    .string = NULL,
    .object_id = O_HARPOON_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 7,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_M16Ammo = {
    .string = NULL,
    .object_id = O_M16_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_GrenadeAmmo = {
    .string = NULL,
    .object_id = O_GRENADE_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 61728,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 6,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_SmallMedi = {
    .string = NULL,
    .object_id = O_SMALL_MEDIPACK_OPTION,
    .frames_total = 26,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 25,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4032,
    .x_rot_pt = 0,
    .x_rot_sel = 58240,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 61440,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 216,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 11,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_LargeMedi = {
    .string = NULL,
    .object_id = O_LARGE_MEDIPACK_OPTION,
    .frames_total = 20,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 19,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3616,
    .x_rot_pt = 0,
    .x_rot_sel = 57376,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 61440,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 352,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 10,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Pickup1 = {
    .string = NULL,
    .object_id = O_PICKUP_OPTION_1,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 111,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Pickup2 = {
    .string = NULL,
    .object_id = O_PICKUP_OPTION_2,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 110,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Puzzle1 = {
    .string = NULL,
    .object_id = O_PUZZLE_OPTION_1,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 108,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Puzzle2 = {
    .string = NULL,
    .object_id = O_PUZZLE_OPTION_2,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 107,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Puzzle3 = {
    .string = NULL,
    .object_id = O_PUZZLE_OPTION_3,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 106,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Puzzle4 = {
    .string = NULL,
    .object_id = O_PUZZLE_OPTION_4,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 105,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Key1 = {
    .string = NULL,
    .object_id = O_KEY_OPTION_1,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 101,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Key2 = {
    .string = NULL,
    .object_id = O_KEY_OPTION_2,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 102,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Key3 = {
    .string = NULL,
    .object_id = O_KEY_OPTION_3,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 103,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Key4 = {
    .string = NULL,
    .object_id = 200,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = 61184,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 104,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Passport = {
    .string = NULL,
    .object_id = 133,
    .frames_total = 30,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 14,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4640,
    .x_rot_pt = 0,
    .x_rot_sel = 61216,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 384,
    .z_trans = 0,
    .meshes_sel = 19,
    .meshes_drawn = 19,
    .inv_pos = 0,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Graphics = {
    .string = NULL,
    .object_id = 153,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4224,
    .x_rot_pt = 0,
    .x_rot_sel = 58304,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 16,
    .y_trans = 0,
    .z_trans_sel = 444,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 1,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Sound = {
    .string = NULL,
    .object_id = 154,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4832,
    .x_rot_pt = 0,
    .x_rot_sel = 60128,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 62464,
    .y_rot = 0,
    .y_trans_sel = 4294967294,
    .y_trans = 0,
    .z_trans_sel = 350,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 2,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Controls = {
    .string = NULL,
    .object_id = 155,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 5504,
    .x_rot_pt = 0,
    .x_rot_sel = 62976,
    .x_rot_nosel = 5632,
    .x_rot = 5632,
    .y_rot_sel = 13312,
    .y_rot = 0,
    .y_trans_sel = 46,
    .y_trans = 0,
    .z_trans_sel = 508,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

INVENTORY_ITEM g_Inv_Item_Photo = {
    .string = NULL,
    .object_id = 122,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4640,
    .x_rot_pt = 0,
    .x_rot_sel = 61216,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 384,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
    .sprite_list = NULL,
    .reserved = { 0, 0, 0, 0 },
};

static char m_LoadGameRequesterStrings1[MAX_LEVELS][50];
static char m_LoadGameRequesterStrings2[MAX_LEVELS][50];

REQUEST_INFO g_LoadGameRequester = {
    .no_selector = 0,
    .ready = 0,
    .pad = 0,
    .items_count = 1,
    .selected = 0,
    .visible_count = 5,
    .line_offset = 0,
    .line_old_offset = 0,
    .pix_width = 296,
    .line_height = 18,
    .x_pos = 0,
    .y_pos = -32,
    .z_pos = 0,
    .item_string_len = 50,
    .pitem_strings1 = (char *)m_LoadGameRequesterStrings1,
    .pitem_strings2 = (char *)m_LoadGameRequesterStrings2,
    .pitem_flags1 = NULL,
    .pitem_flags2 = NULL,
    .heading_flags1 = 0,
    .heading_flags2 = 0,
    .background_flags = 0,
    .moreup_flags = 0,
    .moredown_flags = 0,
    .item_flags1 = { 0 },
    .item_flags2 = { 0 },
    .heading_text1 = NULL,
    .heading_text2 = NULL,
    .background_text = NULL,
    .moreup_text = NULL,
    .moredown_text = NULL,
    .item_texts1 = { NULL },
    .item_texts2 = { NULL },
    .heading_string1 = { 0 },
    .heading_string2 = { 0 },
    .render_width = 0,
    .render_height = 0,
};

REQUEST_INFO g_SaveGameRequester = {
    .no_selector = 0,
    .ready = 0,
    .pad = 0,
    .items_count = 1,
    .selected = 0,
    .visible_count = 5,
    .line_offset = 0,
    .line_old_offset = 0,
    .pix_width = 272,
    .line_height = 18,
    .x_pos = 0,
    .y_pos = -32,
    .z_pos = 0,
    .item_string_len = 50,
    .pitem_strings1 = (char *)g_ValidLevelStrings1,
    .pitem_strings2 = (char *)g_ValidLevelStrings2,
    .pitem_flags1 = NULL,
    .pitem_flags2 = NULL,
    .heading_flags1 = 0,
    .heading_flags2 = 0,
    .background_flags = 0,
    .moreup_flags = 0,
    .moredown_flags = 0,
    .item_flags1 = { 0 },
    .item_flags2 = { 0 },
    .heading_text1 = NULL,
    .heading_text2 = NULL,
    .background_text = NULL,
    .moreup_text = NULL,
    .moredown_text = NULL,
    .item_texts1 = { NULL },
    .item_texts2 = { NULL },
    .heading_string1 = { 0 },
    .heading_string2 = { 0 },
    .render_width = 0,
    .render_height = 0,
};

char **g_GF_CutsceneFileNames = NULL;
char **g_GF_FMVFilenames = NULL;
char **g_GF_GameStrings = NULL;
char **g_GF_Key1Strings = NULL;
char **g_GF_Key2Strings = NULL;
char **g_GF_Key3Strings = NULL;
char **g_GF_Key4Strings = NULL;
char **g_GF_LevelFileNames = NULL;
char **g_GF_LevelNames = NULL;
char **g_GF_PCStrings = NULL;
char **g_GF_PicFilenames = NULL;
char **g_GF_Pickup1Strings = NULL;
char **g_GF_Pickup2Strings = NULL;
char **g_GF_Puzzle1Strings = NULL;
char **g_GF_Puzzle2Strings = NULL;
char **g_GF_Puzzle3Strings = NULL;
char **g_GF_Puzzle4Strings = NULL;
char **g_GF_TitleFileNames = NULL;
char *g_GF_CutsceneFileNamesBuf = NULL;
char *g_GF_FMVFilenamesBuf = NULL;
char *g_GF_GameStringsBuf = NULL;
char *g_GF_Key1StringsBuf = NULL;
char *g_GF_Key2StringsBuf = NULL;
char *g_GF_Key3StringsBuf = NULL;
char *g_GF_Key4StringsBuf = NULL;
char *g_GF_LevelFileNamesBuf = NULL;
char *g_GF_LevelNamesBuf = NULL;
char *g_GF_PCStringsBuf = NULL;
char *g_GF_PicFilenamesBuf = NULL;
char *g_GF_Pickup1StringsBuf = NULL;
char *g_GF_Pickup2StringsBuf = NULL;
char *g_GF_Puzzle1StringsBuf = NULL;
char *g_GF_Puzzle2StringsBuf = NULL;
char *g_GF_Puzzle3StringsBuf = NULL;
char *g_GF_Puzzle4StringsBuf = NULL;
char *g_GF_TitleFileNamesBuf = NULL;
char g_GF_Add2InvItems[GF_ADD_INV_NUMBER_OF] = {};
char g_GF_Description[256] = {};
char g_GF_Kill2Complete = {};
char g_GF_SecretInvItems[GF_ADD_INV_NUMBER_OF] = {};

bool g_GF_DeadlyWater = false;
bool g_GF_RemoveAmmo = false;
bool g_GF_RemoveWeapons = false;
bool g_GF_SunsetEnabled = false;
int16_t *g_GF_FrontendSequence = NULL;
int16_t *g_GF_ScriptTable[MAX_LEVELS] = {};
int16_t *g_GF_SequenceBuf = NULL;
int16_t g_GF_LevelOffsets[200] = {};
int16_t g_GF_MusicTracks[16] = {};
int16_t g_GF_NoFloor = 0;
int16_t g_GF_NumSecrets = 3;
int16_t g_GF_ValidDemos[MAX_DEMO_FILES] = {};
int32_t g_GF_LaraStartAnim;
int32_t g_GF_ScriptVersion;

GAMEMODE g_GameMode;
TEXTSTRING *g_Inv_ItemText[IT_NUMBER_OF] = {};
TEXTSTRING *g_Inv_LevelText = NULL;
TEXTSTRING *g_Inv_TagText = NULL;
int32_t g_SavedGames;
TEXTSTRING *g_PasswordText1 = NULL;
int32_t g_PassportMode;
TEXTSTRING *g_SoundText[4] = {};
char g_ValidLevelStrings1[MAX_LEVELS][50];
char g_ValidLevelStrings2[MAX_LEVELS][50];
uint32_t g_RequesterFlags1[MAX_REQUESTER_ITEMS];
uint32_t g_RequesterFlags2[MAX_REQUESTER_ITEMS];
uint32_t g_SaveGameReqFlags1[MAX_REQUESTER_ITEMS];
uint32_t g_SaveGameReqFlags2[MAX_REQUESTER_ITEMS];
int32_t g_SaveCounter;
int16_t g_SavedLevels[MAX_LEVELS] = { -1, 0 };
int16_t g_Inv_MainQtys[23] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 };
int16_t g_Inv_KeysQtys[23] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 };
uint16_t g_InvColors[INV_COLOR_NUMBER_OF];

int16_t g_MovableBlockBounds[12] = {
    -300, 300, 0, 0, -692, -512, -1820, 1820, -5460, 5460, -1820, 1820,
};
int16_t g_ZiplineHandleBounds[12] = {
    -256, 256, -100, 100, 256, 512, 0, 0, -4550, 4550, 0,
};
int16_t g_PickupBounds[12] = {
    -256, 256, -100, 100, -256, 256, -1820, 1820, 0, 0,
};
int16_t g_GongBounds[12] = {
    -512, 1024, -100, 100, -812, -412, -5460, 5460, 0, 0, 0, 0,
};
int16_t g_PickupBoundsUW[12] = {
    -512, 512, -512, 512, -512, 512, -8190, 8190, -8190, 8190, -8190, 8190,
};
int16_t g_SwitchBounds[12] = {
    -220, 220, 0, 0, 292, 512, -1820, 1820, -5460, 5460, -1820, 1820,
};
int16_t g_SwitchBoundsUW[12] = {
    -1024,  1024,  -1024,  1024,  -1024,  512,
    -14560, 14560, -14560, 14560, -14560, 14560,
};
int16_t g_KeyholeBounds[12] = {
    -200, 200, 0, 0, 312, 512, -1820, 1820, -5460, 5460, -1820, 1820,
};
int16_t g_PuzzleHoleBounds[12] = {
    -200, 200, 0, 0, 312, 512, -1820, 1820, -5460, 5460, -1820, 1820,
};

XYZ_32 g_ZiplineHandlePosition = { 0, 0, 371 };
XYZ_32 g_PickupPosition = { 0, 0, -100 };
XYZ_32 g_PickupPositionUW = { 0, -200, -350 };
XYZ_32 g_SmallSwitchPosition = { 0, 0, 362 };
XYZ_32 g_PushSwitchPosition = { 0, 0, 292 };
XYZ_32 g_AirlockPosition = { 0, 0, 212 };
XYZ_32 g_SwitchUWPosition = { 0, 0, 108 };
XYZ_32 g_KeyholePosition = { 0, 0, 362 };
XYZ_32 g_PuzzleHolePosition = { 0, 0, 327 };
XYZ_32 g_InteractPosition = {};
XYZ_32 g_DetonatorPosition = {};

size_t g_GameBuf_MemCap = 0;
char *g_GameBuf_MemPtr = NULL;
size_t g_GameBuf_MemUsed = 0;
size_t g_GameBuf_MemFree = 0;
RGB_888 g_PicturePalette[256];
bool g_DetonateAllMines = false;
int32_t g_SavegameBufPos = 0;
char *g_SavegameBufPtr = NULL;
ROOM_LIGHT_TABLE g_RoomLightTables[WIBBLE_SIZE] = {};
LIGHT g_DynamicLights[MAX_DYNAMIC_LIGHTS] = {};
int32_t g_RoomLightShades[4] = {};
int32_t g_SunsetTimer = 0;
