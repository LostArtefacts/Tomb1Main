#include "global/vars.h"

#include "game/spawn.h"

#include <libtrx/game/sound/ids.h>

#ifndef MESON_BUILD
const char *g_TRXVersion = "TR2X (non-Docker build)";
#endif

int16_t g_RoomsToDraw[MAX_ROOMS_TO_DRAW] = {};
int16_t g_RoomsToDrawCount = 0;

const float g_RhwFactor = 0x14000000.p0;
uint16_t *g_TexturePageBuffer16[MAX_TEXTURE_PAGES] = {};

SDL_Window *g_SDLWindow = NULL;

uint32_t g_PerspectiveDistance = 0x3000000;
uint32_t g_AssaultBestTime = -1;

int16_t g_CineTargetAngle = DEG_90;
int32_t g_OverlayStatus = 1;
bool g_GymInvOpenEnabled = true; // TODO: make me configurable
int32_t g_MidSort = 0;
GOURAUD_ENTRY g_GouraudTable[256];
int32_t g_PhdWinTop;
int32_t g_LsAdder;
float g_FltWinBottom;
float g_FltResZBuf;
float g_FltResZ;
int32_t g_PhdWinHeight;
int32_t g_PhdWinCenterX;
int32_t g_PhdWinCenterY;
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
float g_FltPersp;
int16_t *g_Info3DPtr = NULL;
int32_t g_PhdWinWidth;
int32_t g_PhdViewDistance;
DEPTHQ_ENTRY g_DepthQTable[32];
int32_t g_LsDivider;
PHD_VBUF g_PhdVBuf[1500];
uint8_t *g_TexturePageBuffer8[MAX_TEXTURE_PAGES] = {};
float g_FltWinRight;
XYZ_32 g_LsVectorView;
int32_t g_PhdWinRight;
int32_t g_SurfaceCount;
SORT_ITEM *g_Sort3DPtr = NULL;
int32_t g_WibbleOffset;
bool g_IsWibbleEffect;
bool g_IsWaterEffect;
bool g_IsShadeEffect;
int32_t g_IsChunkyCamera;
int32_t g_FlipTimer;
bool g_IsDemoLoaded;
bool g_IsAssaultTimerDisplay;
bool g_IsAssaultTimerActive;
bool g_IsMonkAngry;
uint16_t g_SoundOptionLine;
ASSAULT_STATS g_Assault;
int32_t g_LevelItemCount;
int32_t g_HealthBarTimer;
int32_t g_LevelComplete;
RGB_888 g_GamePalette8[256];
RGB_888 g_GamePalette16[256];
SAVEGAME_INFO g_SaveGame;
LARA_INFO g_Lara;
ITEM *g_LaraItem = NULL;
int16_t g_NextItemActive;
int16_t g_PrevItemActive;
int32_t g_SoundEffectCount;
OBJECT g_Objects[265] = {};
int32_t g_RoomCount;
ROOM *g_Rooms = NULL;
int32_t g_FlipStatus;
ITEM *g_Items = NULL;
int16_t g_NumCineFrames;
CINE_FRAME *g_CineData = NULL;
PHD_3DPOS g_CinePos;
int16_t g_CineFrameIdx;
BOX_INFO *g_Boxes = NULL;
int16_t *g_FlyZone[2] = {};
int16_t *g_GroundZone[4][2] = {};
uint16_t *g_Overlap = NULL;
CREATURE *g_BaddieSlots = NULL;

STATIC_OBJECT_3D g_StaticObjects3D[MAX_STATIC_OBJECTS];
STATIC_OBJECT_2D g_StaticObjects2D[MAX_STATIC_OBJECTS];
OBJECT_VECTOR *g_SoundEffects = NULL;
int16_t g_SampleLUT[SFX_NUMBER_OF];
SAMPLE_INFO *g_SampleInfos = NULL;
int32_t g_HeightType;
int32_t g_FlipMaps[MAX_FLIP_MAPS];
bool g_CameraUnderwater;
int32_t g_BoxCount;
int32_t g_TexturePageCount;
int32_t g_NumCameras;
uint32_t *g_DemoData = NULL;
char g_LevelFileName[256];
uint16_t g_MusicTrackFlags[64];

WEAPON_INFO g_Weapons[] = {
    {},
    {
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -60 * DEG_1, 60 * DEG_1 },
        .left_angles = { -170 * DEG_1, 60 * DEG_1, -80 * DEG_1, 80 * DEG_1 },
        .right_angles = { -60 * DEG_1, 170 * DEG_1, -80 * DEG_1, 80 * DEG_1 },
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
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -60 * DEG_1, 60 * DEG_1 },
        .left_angles = { -170 * DEG_1, 60 * DEG_1, -80 * DEG_1, 80 * DEG_1 },
        .right_angles = { -60 * DEG_1, 170 * DEG_1, -80 * DEG_1, 80 * DEG_1 },
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
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -60 * DEG_1, 60 * DEG_1 },
        .left_angles = { -170 * DEG_1, 60 * DEG_1, -80 * DEG_1, 80 * DEG_1 },
        .right_angles = { -60 * DEG_1, 170 * DEG_1, -80 * DEG_1, 80 * DEG_1 },
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
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -55 * DEG_1, 55 * DEG_1 },
        .left_angles = { -80 * DEG_1, 80 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
        .right_angles = { -80 * DEG_1, 80 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
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
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -55 * DEG_1, 55 * DEG_1 },
        .left_angles = { -80 * DEG_1, 80 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
        .right_angles = { -80 * DEG_1, 80 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
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
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -55 * DEG_1, 55 * DEG_1 },
        .left_angles = { -80 * DEG_1, 80 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
        .right_angles = { -80 * DEG_1, 80 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
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
        .lock_angles = { -60 * DEG_1, 60 * DEG_1, -65 * DEG_1, 65 * DEG_1 },
        .left_angles = { -80 * DEG_1, 80 * DEG_1, -75 * DEG_1, 75 * DEG_1 },
        .right_angles = { -80 * DEG_1, 80 * DEG_1, -75 * DEG_1, 75 * DEG_1 },
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
        .lock_angles = { -30 * DEG_1, 30 * DEG_1, -55 * DEG_1, 55 * DEG_1 },
        .left_angles = { -30 * DEG_1, 30 * DEG_1, -55 * DEG_1, 55 * DEG_1 },
        .right_angles = { -30 * DEG_1, 30 * DEG_1, -55 * DEG_1, 55 * DEG_1 },
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
    .item_flags1 = {},
    .item_flags2 = {},
    .heading_text1 = NULL,
    .heading_text2 = NULL,
    .background_text = NULL,
    .moreup_text = NULL,
    .moredown_text = NULL,
    .item_texts1 = { NULL },
    .item_texts2 = { NULL },
    .heading_string1 = {},
    .heading_string2 = {},
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
    .item_flags1 = {},
    .item_flags2 = {},
    .heading_text1 = NULL,
    .heading_text2 = NULL,
    .background_text = NULL,
    .moreup_text = NULL,
    .moredown_text = NULL,
    .item_texts1 = { NULL },
    .item_texts2 = { NULL },
    .heading_string1 = {},
    .heading_string2 = {},
    .render_width = 0,
    .render_height = 0,
};

bool g_GF_RemoveAmmo = false;
bool g_GF_RemoveWeapons = false;
bool g_GF_SunsetEnabled = false;
int16_t g_GF_NoFloor = 0;
int16_t g_GF_NumSecrets = 3;
int32_t g_GF_LaraStartAnim;
int32_t g_GF_ScriptVersion;

int32_t g_SavedGames;
TEXTSTRING *g_PasswordText1 = NULL;
char g_ValidLevelStrings1[MAX_LEVELS][50];
char g_ValidLevelStrings2[MAX_LEVELS][50];
uint32_t g_RequesterFlags1[MAX_REQUESTER_ITEMS];
uint32_t g_RequesterFlags2[MAX_REQUESTER_ITEMS];
int32_t g_SaveCounter;
int16_t g_SavedLevels[MAX_LEVELS] = { -1, 0 };

XYZ_32 g_InteractPosition = { .x = 0, .y = 0, .z = 0 };
bool g_DetonateAllMines = false;
int32_t g_SunsetTimer = 0;

NAMED_COLOR g_NamedColors[COLOR_NUMBER_OF] = {
    // clang-format off
    [COLOR_BLACK]      = {.rgb = {.r = 0x00, .g = 0x00, .b = 0x00}},
    [COLOR_GRAY]       = {.rgb = {.r = 0x40, .g = 0x40, .b = 0x40}},
    [COLOR_WHITE]      = {.rgb = {.r = 0xFF, .g = 0xFF, .b = 0xFF}},
    [COLOR_RED]        = {.rgb = {.r = 0xFF, .g = 0x00, .b = 0x00}},
    [COLOR_ORANGE]     = {.rgb = {.r = 0xFF, .g = 0x80, .b = 0x00}},
    [COLOR_YELLOW]     = {.rgb = {.r = 0xFF, .g = 0xFF, .b = 0x00}},
    [COLOR_DARK_GREEN] = {.rgb = {.r = 0x00, .g = 0x80, .b = 0x00}},
    [COLOR_GREEN]      = {.rgb = {.r = 0x00, .g = 0xFF, .b = 0x00}},
    [COLOR_CYAN]       = {.rgb = {.r = 0x00, .g = 0xFF, .b = 0xFF}},
    [COLOR_BLUE]       = {.rgb = {.r = 0x00, .g = 0x00, .b = 0xFF}},
    [COLOR_MAGENTA]    = {.rgb = {.r = 0xFF, .g = 0x00, .b = 0xFF}},
    // clang-format on
};
