#pragma once

#include "global/types.h"

#include <stdint.h>
#include <windows.h>

// TODO: This is a placeholder file containing functions that originated from
// the decompilation phase, and they are currently disorganized. Eventually,
// they'll need to be properly modularized. The same applies to all files
// within the decomp/ directory which are scheduled for extensive refactoring.

int32_t __cdecl GameInit(void);
int32_t __stdcall WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
    int32_t nShowCmd);
const char *__cdecl DecodeErrorMessage(int32_t error_code);
int32_t __cdecl RenderErrorBox(int32_t error_code);
bool __cdecl DInputCreate(void);
void __cdecl DInputRelease(void);
void __cdecl WinInReadKeyboard(uint8_t *input_data);
int32_t __cdecl WinGameStart(void);
int16_t __cdecl TitleSequence(void);
void __cdecl WinVidSetMinWindowSize(int32_t width, int32_t height);
void __cdecl WinVidSetMaxWindowSize(int32_t width, int32_t height);
void __cdecl WinVidClearMinWindowSize(void);
void __cdecl WinVidClearMaxWindowSize(void);
int32_t __cdecl CalculateWindowWidth(int32_t width, int32_t height);
int32_t __cdecl CalculateWindowHeight(int32_t width, int32_t height);
bool __cdecl WinVidGetMinMaxInfo(LPMINMAXINFO info);
HWND __cdecl WinVidFindGameWindow(void);
bool __cdecl WinVidSpinMessageLoop(bool need_wait);
void __cdecl WinVidShowGameWindow(int32_t cmd_show);
void __cdecl WinVidHideGameWindow(void);
void __cdecl WinVidSetGameWindowSize(int32_t width, int32_t height);
bool __cdecl ShowDDrawGameWindow(bool active);
bool __cdecl HideDDrawGameWindow(void);
HRESULT __cdecl DDrawSurfaceCreate(LPDDSDESC dsp, LPDDS *surface);
HRESULT __cdecl DDrawSurfaceRestoreLost(
    LPDDS surface1, LPDDS surface2, bool blank);
bool __cdecl WinVidClearBuffer(LPDDS surface, LPRECT rect, DWORD fill_color);
HRESULT __cdecl WinVidBufferLock(LPDDS surface, LPDDSDESC desc, DWORD flags);
HRESULT __cdecl WinVidBufferUnlock(LPDDS surface, LPDDSDESC desc);
bool __cdecl WinVidCopyBitmapToBuffer(LPDDS surface, const BYTE *bitmap);
DWORD __cdecl GetRenderBitDepth(uint32_t rgb_bit_count);
void __thiscall WinVidGetColorBitMasks(
    COLOR_BIT_MASKS *bm, LPDDPIXELFORMAT pixel_format);
void __cdecl BitMaskGetNumberOfBits(
    uint32_t bit_mask, uint32_t *bit_depth, uint32_t *bit_offset);
DWORD __cdecl CalculateCompatibleColor(
    const COLOR_BIT_MASKS *mask, int32_t red, int32_t green, int32_t blue,
    int32_t alpha);
bool __cdecl WinVidGetDisplayMode(DISPLAY_MODE *disp_mode);
bool __cdecl WinVidGoFullScreen(DISPLAY_MODE *disp_mode);
bool __cdecl WinVidGoWindowed(
    int32_t width, int32_t height, DISPLAY_MODE *disp_mode);
void __cdecl WinVidSetDisplayAdapter(DISPLAY_ADAPTER *disp_adapter);
void __cdecl Game_SetCutsceneTrack(int32_t track);
int32_t __cdecl Game_Cutscene_Start(int32_t level_num);
void __cdecl Misc_InitCinematicRooms(void);
int32_t __cdecl Game_Cutscene_Control(int32_t nframes);
void __cdecl CutscenePlayer_Control(int16_t item_num);
void __cdecl Lara_Control_Cutscene(int16_t item_num);
void __cdecl CutscenePlayer1_Initialise(int16_t item_num);
void __cdecl CutscenePlayerGen_Initialise(int16_t item_num);
int32_t __cdecl Level_Initialise(
    int32_t level_num, GAMEFLOW_LEVEL_TYPE level_type);
void __cdecl CreateScreenBuffers(void);
void __cdecl CreatePrimarySurface(void);
void __cdecl CreateBackBuffer(void);
void __cdecl CreateClipper(void);
void __cdecl CreateWindowPalette(void);
void __cdecl CreateZBuffer(void);
int32_t __cdecl GetZBufferDepth(void);
void __cdecl CreateRenderBuffer(void);
void __cdecl CreatePictureBuffer(void);
void __cdecl ClearBuffers(DWORD flags, DWORD fill_color);
void __cdecl UpdateFrame(bool need_run_message_loop, LPRECT rect);
void __cdecl RestoreLostBuffers(void);
void __cdecl WaitPrimaryBufferFlip(void);
bool __cdecl RenderInit(void);
void __cdecl RenderStart(bool is_reset);
void __cdecl RenderFinish(bool need_to_clear_textures);
bool __cdecl ApplySettings(const APP_SETTINGS *new_settings);
void __cdecl GameApplySettings(APP_SETTINGS *new_settings);
void __cdecl UpdateGameResolution(void);
bool __cdecl D3DCreate(void);
void __cdecl D3DRelease(void);
void __cdecl Enumerate3DDevices(DISPLAY_ADAPTER *adapter);
HRESULT __stdcall Enum3DDevicesCallback(
    GUID FAR *lpGuid, LPTSTR lpDeviceDescription, LPTSTR lpDeviceName,
    LPD3DDEVICEDESC_V2 lpD3DHWDeviceDesc, LPD3DDEVICEDESC_V2 lpD3DHELDeviceDesc,
    LPVOID lpContext);
bool __cdecl D3DIsSupported(LPD3DDEVICEDESC_V2 desc);
bool __cdecl D3DSetViewport(void);
void __cdecl D3DDeviceCreate(LPDDS lpBackBuffer);
void __cdecl Direct3DRelease(void);
bool __cdecl Direct3DInit(void);
bool __cdecl DDrawCreate(LPGUID lpGUID);
void __cdecl DDrawRelease(void);
void __cdecl GameWindowCalculateSizeFromClient(int32_t *width, int32_t *height);
void __cdecl GameWindowCalculateSizeFromClientByZero(
    int32_t *width, int32_t *height);
bool __thiscall CompareVideoModes(
    const DISPLAY_MODE *mode1, const DISPLAY_MODE *mode2);
bool __cdecl WinVidGetDisplayModes(void);
HRESULT __stdcall EnumDisplayModesCallback(
    LPDDSDESC lpDDSurfaceDesc, LPVOID lpContext);
bool __cdecl WinVidInit(void);
bool __cdecl WinVidGetDisplayAdapters(void);
bool __cdecl EnumerateDisplayAdapters(
    DISPLAY_ADAPTER_LIST *display_adapter_list);
bool __cdecl WinVidRegisterGameWindowClass(void);
LRESULT CALLBACK
WinVidGameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI EnumDisplayAdaptersCallback(
    GUID FAR *lpGUID, LPTSTR lpDriverDescription, LPTSTR lpDriverName,
    LPVOID lpContext);
void __cdecl WinVidResizeGameWindow(HWND hWnd, int32_t edge, LPRECT rect);
bool __cdecl WinVidCheckGameWindowPalette(HWND hWnd);
bool __cdecl WinVidCreateGameWindow(void);
void __cdecl WinVidFreeWindow(void);
void __cdecl WinVidExitMessage(void);
DISPLAY_ADAPTER_NODE *__cdecl WinVidGetDisplayAdapter(const GUID *guid_ptr);
void __cdecl WinVidStart(void);
void __cdecl WinVidFinish(void);
int32_t __cdecl Misc_Move3DPosTo3DPos(
    PHD_3DPOS *src_pos, const PHD_3DPOS *dst_pos, int32_t velocity,
    PHD_ANGLE ang_add);
int32_t __cdecl LevelCompleteSequence(void);
void __cdecl S_LoadSettings(void);
void __cdecl S_SaveSettings(void);
void __cdecl S_Wait(int32_t frames, BOOL input_check);
BOOL __cdecl S_InitialiseSystem(void);
void __cdecl S_DisplayPicture(const char *file_name, BOOL is_title);
void __cdecl DisplayCredits(void);
DWORD __cdecl S_DumpScreen(void);
int32_t __cdecl GetRenderHeight(void);
int32_t __cdecl GetRenderWidth(void);
void __cdecl S_InitialisePolyList(bool clear_back_buffer);
void __cdecl S_ClearScreen(void);
void __cdecl S_InitialiseScreen(GAMEFLOW_LEVEL_TYPE level_type);
void __cdecl S_OutputPolyList(void);
void __cdecl S_InsertBackPolygon(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void __cdecl S_DrawScreenLine(
    int32_t x, int32_t y, int32_t z, int32_t x_len, int32_t y_len,
    uint8_t color_idx, const D3DCOLOR *gour, uint16_t flags);
void __cdecl S_DrawScreenBox(
    int32_t sx, int32_t sy, int32_t z, int32_t width, int32_t height,
    uint8_t color_idx, const GOURAUD_OUTLINE *gour, uint16_t flags);
void __cdecl S_DrawScreenFBox(
    int32_t sx, int32_t sy, int32_t z, int32_t width, int32_t height,
    uint8_t color_idx, const GOURAUD_FILL *gour, uint16_t flags);
void __cdecl S_FinishInventory(void);
void __cdecl S_FadeToBlack(void);
uint16_t __cdecl S_FindColor(int32_t red, int32_t green, int32_t blue);
void __cdecl S_CopyBufferToScreen(void);
void __cdecl DecreaseScreenSize(void);
void __cdecl IncreaseScreenSize(void);
void __cdecl TempVideoAdjust(int32_t hires, double sizer);
void __cdecl TempVideoRemove(void);
void __cdecl setup_screen_size(void);
void __cdecl S_FadeInInventory(bool is_fade);
void __cdecl S_FadeOutInventory(bool is_fade);
void __cdecl CopyBitmapPalette(
    const RGB_888 *src_pal, const uint8_t *src_bitmap, int32_t bitmap_size,
    RGB_888 *out_pal);
uint8_t __cdecl FindNearestPaletteEntry(
    const RGB_888 *palette, int32_t red, int32_t green, int32_t blue,
    bool ignore_sys_palette);
void __cdecl SyncSurfacePalettes(
    const void *src_data, int32_t width, int32_t height, int32_t src_pitch,
    const RGB_888 *src_palette, void *dst_data, int32_t dst_pitch,
    const RGB_888 *dst_palette, bool preserve_sys_palette);
int32_t __cdecl CreateTexturePalette(const RGB_888 *palette);
int32_t __cdecl GetFreePaletteIndex(void);
void __cdecl FreePalette(int32_t palette_idx);
void __cdecl SafeFreePalette(int32_t palette_idx);
int32_t __cdecl CreateTexturePage(
    int32_t width, int32_t height, LPDIRECTDRAWPALETTE palette);
int32_t __cdecl GetFreeTexturePageIndex(void);
bool __cdecl CreateTexturePageSurface(TEXPAGE_DESC *desc);
bool __cdecl TexturePageInit(TEXPAGE_DESC *page);
LPDIRECT3DTEXTURE2 __cdecl Create3DTexture(const LPDDS surface);
void __cdecl SafeFreeTexturePage(int32_t page_idx);
void __cdecl FreeTexturePage(int32_t page_idx);
void __cdecl TexturePageReleaseVidMemSurface(TEXPAGE_DESC *page);
void __cdecl FreeTexturePages(void);
bool __cdecl LoadTexturePage(int32_t page_idx, bool reset);
bool __cdecl ReloadTextures(bool reset);
HWR_TEXTURE_HANDLE __cdecl GetTexturePageHandle(int32_t page_idx);
int32_t __cdecl AddTexturePage8(
    int32_t width, int32_t height, const uint8_t *page_buf, int32_t pal_idx);
int32_t __cdecl AddTexturePage16(
    int32_t width, int32_t height, const uint8_t *page_buf);
HRESULT __stdcall EnumTextureFormatsCallback(LPDDSDESC desc, LPVOID lpContext);
HRESULT __cdecl EnumerateTextureFormats(void);
void __cdecl CleanupTextures(void);
bool __cdecl InitTextures(void);
void __cdecl S_SyncPictureBufferPalette(void);
void __cdecl S_DontDisplayPicture(void);
void __cdecl ScreenDump(void);
void __cdecl ScreenPartialDump(void);
void __cdecl FadeToPal(int32_t fade_value, const RGB_888 *palette);
void __cdecl ScreenClear(bool is_phd_win_size);
void __cdecl S_CopyScreenToBuffer(void);
void __cdecl AdjustTextureUVs(bool reset_uv_add);
void __cdecl S_AdjustTexelCoordinates(void);
