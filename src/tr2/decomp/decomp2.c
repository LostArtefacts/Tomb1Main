#include "decomp/decomp.h"
#include "game/background.h"
#include "game/hwr.h"
#include "game/inventory/common.h"
#include "game/level.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>
#include <libtrx/virtual_file.h>

int32_t __cdecl CreateTexturePage(
    const int32_t width, const int32_t height, LPDIRECTDRAWPALETTE palette)
{
    const int32_t palette_idx = GetFreeTexturePageIndex();
    if (palette_idx < 0) {
        return -1;
    }
    TEXPAGE_DESC *const page = &g_TexturePages[palette_idx];
    memset(page, 0, sizeof(TEXPAGE_DESC));
    page->status = 1;
    page->width = width;
    page->height = height;
    page->palette = palette;
    if (!CreateTexturePageSurface(page)) {
        return -1;
    }

    TexturePageInit((TEXPAGE_DESC *)&g_TexturePages[palette_idx]);
    return palette_idx;
}

int32_t __cdecl GetFreeTexturePageIndex(void)
{
    for (int32_t i = 0; i < MAX_TEXTURE_PAGES; i++) {
        if (!(g_TexturePages[i].status & 1)) {
            return i;
        }
    }
    return -1;
}

bool __cdecl CreateTexturePageSurface(TEXPAGE_DESC *const page)
{
    DDSURFACEDESC dsp = { 0 };
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    dsp.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    dsp.dwWidth = page->width;
    dsp.dwHeight = page->height;
    dsp.ddpfPixelFormat = g_TextureFormat.pixel_fmt;

    if (FAILED(DDrawSurfaceCreate(&dsp, &page->sys_mem_surface))) {
        return false;
    }

    if (page->palette != NULL
        && FAILED(page->sys_mem_surface->lpVtbl->SetPalette(
            page->sys_mem_surface, page->palette))) {
        return false;
    }

    return true;
}

bool __cdecl TexturePageInit(TEXPAGE_DESC *const page)
{
    bool result = false;

    DDSURFACEDESC dsp = { 0 };
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    dsp.dwWidth = page->width;
    dsp.dwHeight = page->height;
    dsp.ddpfPixelFormat = g_TextureFormat.pixel_fmt;
    dsp.ddsCaps.dwCaps =
        DDSCAPS_ALLOCONLOAD | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;

    if (FAILED(DDrawSurfaceCreate(&dsp, &page->vid_mem_surface))) {
        return false;
    }

    if (page->palette != NULL) {
        if (FAILED(page->vid_mem_surface->lpVtbl->SetPalette(
                page->vid_mem_surface, page->palette))) {
            goto cleanup;
        }

        DDCOLORKEY color_key;
        color_key.dwColorSpaceLowValue = 0;
        color_key.dwColorSpaceHighValue = 0;
        if (FAILED(page->vid_mem_surface->lpVtbl->SetColorKey(
                page->vid_mem_surface, DDCKEY_SRCBLT, &color_key))) {
            goto cleanup;
        }
    }

    page->texture_3d = Create3DTexture(page->vid_mem_surface);
    if (page->texture_3d == NULL) {
        goto cleanup;
    }

    if (FAILED(page->texture_3d->lpVtbl->GetHandle(
            page->texture_3d, g_D3DDev, &page->tex_handle))) {
        goto cleanup;
    }

    result = true;

cleanup:
    if (!result) {
        if (page->texture_3d != NULL) {
            page->texture_3d->lpVtbl->Release(page->texture_3d);
            page->texture_3d = NULL;
        }

        if (page->vid_mem_surface != NULL) {
            page->vid_mem_surface->lpVtbl->Release(page->vid_mem_surface);
            page->vid_mem_surface = NULL;
        }
    }

    return result;
}

LPDIRECT3DTEXTURE2 __cdecl Create3DTexture(const LPDDS surface)
{
    LPDIRECT3DTEXTURE2 texture_3d = NULL;
    if (FAILED(surface->lpVtbl->QueryInterface(
            surface, &g_IID_IDirect3DTexture2, (LPVOID *)&texture_3d))) {
        return NULL;
    }
    return texture_3d;
}

void __cdecl SafeFreeTexturePage(const int32_t page_idx)
{
    if (page_idx >= 0 && (g_TexturePages[page_idx].status & 1)) {
        FreeTexturePage(page_idx);
    }
}

void __cdecl FreeTexturePage(const int32_t page_idx)
{
    TEXPAGE_DESC *const page = &g_TexturePages[page_idx];
    TexturePageReleaseVidMemSurface(page);
    if (page->sys_mem_surface != NULL) {
        page->sys_mem_surface->lpVtbl->Release(page->sys_mem_surface);
        page->sys_mem_surface = NULL;
    }
    page->status = 0;
}

void __cdecl TexturePageReleaseVidMemSurface(TEXPAGE_DESC *const page)
{
    HWR_ResetTexSource();
    page->tex_handle = 0;
    if (page->texture_3d != NULL) {
        page->texture_3d->lpVtbl->Release(page->texture_3d);
        page->texture_3d = NULL;
    }
    if (page->vid_mem_surface != NULL) {
        page->vid_mem_surface->lpVtbl->Release(page->vid_mem_surface);
        page->vid_mem_surface = NULL;
    }
}

void __cdecl FreeTexturePages(void)
{
    for (int32_t i = 0; i < MAX_TEXTURE_PAGES; i++) {
        TEXPAGE_DESC *const page = &g_TexturePages[i];
        if (page->status & 1) {
            FreeTexturePage(i);
        }
    }
}

bool __cdecl LoadTexturePage(const int32_t page_idx, const bool reset)
{
    bool rc = false;
    if (page_idx < 0) {
        return false;
    }

    TEXPAGE_DESC *const page = &g_TexturePages[page_idx];
    if (reset || page->vid_mem_surface == NULL) {
        rc = SUCCEEDED(
            DDrawSurfaceRestoreLost(page->vid_mem_surface, NULL, false));
    }

    if (!rc) {
        TexturePageReleaseVidMemSurface(page);
        rc = TexturePageInit(page);
    }

    if (!rc) {
        return false;
    }

    DDrawSurfaceRestoreLost(page->sys_mem_surface, 0, 0);
    LPDIRECT3DTEXTURE2 sys_mem_texture = Create3DTexture(page->sys_mem_surface);
    if (sys_mem_texture == NULL) {
        return false;
    }

    rc = SUCCEEDED(
        page->texture_3d->lpVtbl->Load(page->texture_3d, sys_mem_texture));
    sys_mem_texture->lpVtbl->Release(sys_mem_texture);
    return rc;
}

bool __cdecl ReloadTextures(const bool reset)
{
    bool result = true;
    for (int32_t i = 0; i < MAX_TEXTURE_PAGES; i++) {
        if (g_TexturePages[i].status & 1) {
            result &= LoadTexturePage(i, reset);
        }
    }
    return result;
}

HWR_TEXTURE_HANDLE __cdecl GetTexturePageHandle(const int32_t page_idx)
{
    if (page_idx < 0) {
        return 0;
    }

    TEXPAGE_DESC *const page = &g_TexturePages[page_idx];
    if (page->vid_mem_surface != NULL) {
        if (page->vid_mem_surface->lpVtbl->IsLost(page->vid_mem_surface)
            == DDERR_SURFACELOST) {
            LoadTexturePage(page_idx, true);
        }
    }
    return page->tex_handle;
}

int32_t __cdecl AddTexturePage8(
    const int32_t width, const int32_t height, const uint8_t *const page_buf,
    const int32_t pal_idx)
{
    if (pal_idx < 0) {
        return -1;
    }

    const int32_t page_idx =
        CreateTexturePage(width, height, g_TexturePalettes[pal_idx]);
    if (page_idx < 0) {
        return -1;
    }

    TEXPAGE_DESC *const page = &g_TexturePages[page_idx];

    DDSURFACEDESC desc;
    if (FAILED(WinVidBufferLock(
            page->sys_mem_surface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT))) {
        return -1;
    }

    const uint8_t *src = page_buf;
    uint8_t *dst = desc.lpSurface;
    for (int32_t y = 0; y < height; y++) {
        memcpy(dst, src, width);
        src += width;
        dst += desc.lPitch;
    }

    WinVidBufferUnlock(page->sys_mem_surface, &desc);
    LoadTexturePage(page_idx, false);

    return page_idx;
}

int32_t __cdecl AddTexturePage16(
    const int32_t width, const int32_t height, const uint8_t *const page_buf)
{
    const int32_t page_idx = CreateTexturePage(width, height, NULL);
    if (page_idx < 0) {
        return -1;
    }

    TEXPAGE_DESC *const page = &g_TexturePages[page_idx];

    DDSURFACEDESC desc;
    if (FAILED(WinVidBufferLock(
            page->sys_mem_surface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT))) {
        return -1;
    }

    if (g_TexturesHaveCompatibleMasks) {
        const uint8_t *src = page_buf;
        uint8_t *dst = (uint8_t *)desc.lpSurface;
        for (int32_t y = 0; y < height; y++) {
            memcpy(dst, src, 2 * width);
            src += 2 * width;
            dst += desc.lPitch;
        }
    } else {
        const int32_t bytes_per_pixel = (g_TextureFormat.bpp + 7) >> 3;

        uint8_t *dst = (uint8_t *)desc.lpSurface;
        uint16_t *src = (uint16_t *)page_buf;
        for (int32_t y = 0; y < height; y++) {
            uint8_t *subdst = dst;
            for (int32_t x = 0; x < width; x++) {
                uint32_t compatible_color = CalculateCompatibleColor(
                    &g_TextureFormat.color_bit_masks, (*src >> 7) & 0xF8,
                    (*src >> 2) & 0xF8, (*src << 3) & 0xF8, (*src >> 15) & 1);
                src++;

                for (int32_t k = 0; k < bytes_per_pixel; k++) {
                    *subdst++ = compatible_color;
                    compatible_color >>= 8;
                }
            }

            dst += desc.lPitch;
        }
    }

    WinVidBufferUnlock(page->sys_mem_surface, &desc);
    LoadTexturePage(page_idx, false);

    return page_idx;
}

HRESULT __stdcall EnumTextureFormatsCallback(LPDDSDESC desc, LPVOID lpContext)
{
    DDPIXELFORMAT *pixel_fmt = &desc->ddpfPixelFormat;
    if (pixel_fmt->dwRGBBitCount < 8) {
        return D3DENUMRET_OK;
    }

    if (g_SavedAppSettings.disable_16bit_textures
        || pixel_fmt->dwRGBBitCount != 16) {
        if (pixel_fmt->dwFlags & DDPF_PALETTEINDEXED8) {
            g_TextureFormat.pixel_fmt = *pixel_fmt;
            g_TextureFormat.bpp = 8;
            g_TexturesAlphaChannel = 0;
            g_TexturesHaveCompatibleMasks = false;
            return D3DENUMRET_CANCEL;
        }
    } else if (pixel_fmt->dwFlags & DDPF_RGB) {
        g_TextureFormat.pixel_fmt = *pixel_fmt;
        g_TextureFormat.bpp = 16;
        g_TexturesAlphaChannel = pixel_fmt->dwFlags & DDPF_ALPHAPIXELS;
        WinVidGetColorBitMasks(&g_TextureFormat.color_bit_masks, pixel_fmt);

        if (g_TextureFormat.bpp == 16
            && g_TextureFormat.color_bit_masks.depth.a == 1
            && g_TextureFormat.color_bit_masks.depth.r == 5
            && g_TextureFormat.color_bit_masks.depth.g == 5
            && g_TextureFormat.color_bit_masks.depth.b == 5
            && g_TextureFormat.color_bit_masks.offset.a == 15
            && g_TextureFormat.color_bit_masks.offset.r == 10
            && g_TextureFormat.color_bit_masks.offset.g == 5
            && g_TextureFormat.color_bit_masks.offset.b == 0) {
            g_TexturesHaveCompatibleMasks = true;
            return D3DENUMRET_CANCEL;
        } else {
            g_TexturesHaveCompatibleMasks = false;
            return D3DENUMRET_OK;
        }
    }

    return D3DENUMRET_OK;
}

HRESULT __cdecl EnumerateTextureFormats(void)
{
    memset(&g_TextureFormat, 0, sizeof(g_TextureFormat));
    return g_D3DDev->lpVtbl->EnumTextureFormats(
        g_D3DDev, EnumTextureFormatsCallback, NULL);
}

void __cdecl CleanupTextures(void)
{
    FreeTexturePages();
    for (int32_t i = 0; i < MAX_PALETTES; i++) {
        SafeFreePalette(i);
    }
}

bool __cdecl InitTextures(void)
{
    memset(g_TexturePages, 0, sizeof(TEXPAGE_DESC) * MAX_TEXTURE_PAGES);
    memset(g_TexturePalettes, 0, sizeof(LPDIRECTDRAWPALETTE) * MAX_PALETTES);
    return true;
}

void __cdecl S_SyncPictureBufferPalette(void)
{
    if (g_PictureBufferSurface == NULL) {
        return;
    }

    DDSURFACEDESC desc;
    if (FAILED(WinVidBufferLock(
            g_PictureBufferSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT))) {
        return;
    }

    SyncSurfacePalettes(
        desc.lpSurface, 640, 480, desc.lPitch, g_PicturePalette, desc.lpSurface,
        desc.lPitch, g_GamePalette8, true);
    WinVidBufferUnlock(g_PictureBufferSurface, &desc);
    memcpy(g_PicturePalette, g_GamePalette8, sizeof(RGB_888) * 256);
}

void __cdecl S_DontDisplayPicture(void)
{
    if (g_SavedAppSettings.render_mode == RM_HARDWARE) {
        BGND_Free();
        g_BGND_PictureIsReady = 0;
    }
}

void __cdecl ScreenDump(void)
{
    UpdateFrame(true, NULL);
}

void __cdecl ScreenPartialDump(void)
{
    UpdateFrame(true, &g_PhdWinRect);
}

void __cdecl FadeToPal(const int32_t fade_value, const RGB_888 *const palette)
{
    if (!g_GameVid_IsVga) {
        return;
    }

    int32_t start_idx;
    int32_t end_idx;
    if (g_GameVid_IsWindowedVGA) {
        start_idx = 10;
        end_idx = 246;
    } else {
        start_idx = 0;
        end_idx = 256;
    }
    const int32_t pal_size = end_idx - start_idx;

    if (fade_value <= 1) {
        for (int32_t i = start_idx; i < end_idx; i++) {
            g_WinVid_Palette[i].peRed = palette[i].red;
            g_WinVid_Palette[i].peGreen = palette[i].green;
            g_WinVid_Palette[i].peBlue = palette[i].blue;
        }
        g_DDrawPalette->lpVtbl->SetEntries(
            g_DDrawPalette, 0, start_idx, pal_size,
            &g_WinVid_Palette[start_idx]);
        return;
    }

    PALETTEENTRY fade_pal[256];
    for (int32_t i = start_idx; i < end_idx; i++) {
        fade_pal[i] = g_WinVid_Palette[i];
    }

    for (int32_t j = 0; j <= fade_value; j++) {
        for (int32_t i = start_idx; i < end_idx; i++) {
            g_WinVid_Palette[i].peRed = fade_pal[i].peRed
                + (palette[i].red - fade_pal[i].peRed) * j / fade_value;
            g_WinVid_Palette[i].peGreen = fade_pal[i].peGreen
                + (palette[i].green - fade_pal[i].peGreen) * j / fade_value;
            g_WinVid_Palette[i].peBlue = fade_pal[i].peBlue
                + (palette[i].blue - fade_pal[i].peBlue) * j / fade_value;
        }
        g_DDrawPalette->lpVtbl->SetEntries(
            g_DDrawPalette, 0, start_idx, pal_size,
            &g_WinVid_Palette[start_idx]);
        S_DumpScreen();
    }
}

void __cdecl ScreenClear(const bool is_phd_win_size)
{
    uint32_t flags = (g_SavedAppSettings.render_mode == RM_HARDWARE)
        ? CLRB_BACK_BUFFER
        : CLRB_RENDER_BUFFER;

    if (is_phd_win_size) {
        flags |= CLRB_PHDWINSIZE;
    }

    ClearBuffers(flags, 0);
}

void __cdecl S_CopyScreenToBuffer(void)
{
    if (g_SavedAppSettings.render_mode != RM_SOFTWARE) {
        return;
    }

    g_PictureBufferSurface->lpVtbl->Blt(
        g_PictureBufferSurface, NULL, g_RenderBufferSurface, &g_GameVid_Rect,
        DDBLT_WAIT, NULL);

    DDSURFACEDESC desc;
    if (SUCCEEDED(WinVidBufferLock(
            g_PictureBufferSurface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT))) {
        uint8_t *dst_ptr = desc.lpSurface;
        for (int32_t y = 0; y < 480; y++) {
            for (int32_t x = 0; x < 640; x++) {
                dst_ptr[x] = g_DepthQIndex[dst_ptr[x]];
            }
            dst_ptr += desc.lPitch;
        }
        WinVidBufferUnlock(g_PictureBufferSurface, &desc);
    }

    memcpy(g_PicturePalette, g_GamePalette8, sizeof(RGB_888) * 256);
}

void __cdecl AdjustTextureUVs(const bool reset_uv_add)
{
    if (reset_uv_add) {
        g_UVAdd = 0;
    }

    int32_t adjustment = g_SavedAppSettings.nearest_adjustment;
    if (g_SavedAppSettings.render_mode == RM_HARDWARE
        && (g_SavedAppSettings.texel_adjust_mode == TAM_ALWAYS
            || (g_SavedAppSettings.texel_adjust_mode == TAM_BILINEAR_ONLY
                && g_SavedAppSettings.bilinear_filtering))) {
        adjustment = g_SavedAppSettings.linear_adjustment;
    }

    const int32_t offset = adjustment - g_UVAdd;
    for (int32_t i = 0; i < g_TextureInfoCount; i++) {
        PHD_UV *const uv = g_TextureInfo[i].uv;
        int32_t uv_flags = g_LabTextureUVFlag[i];
        for (int32_t j = 0; j < 4; j++) {
            uv[j].u += (uv_flags & 1) ? -offset : offset;
            uv[j].v += (uv_flags & 2) ? -offset : offset;
            uv_flags >>= 2;
        }
    }

    g_UVAdd += offset;
}

void __cdecl S_AdjustTexelCoordinates(void)
{
    if (g_TextureInfoCount > 0) {
        AdjustTextureUVs(false);
    }
}

BOOL __cdecl S_LoadLevelFile(
    const char *const file_name, const int32_t level_num,
    const GAMEFLOW_LEVEL_TYPE level_type)
{
    S_UnloadLevelFile();
    return Level_Load(file_name, level_num);
}

void __cdecl S_UnloadLevelFile(void)
{
    if (g_SavedAppSettings.render_mode == RM_HARDWARE) {
        HWR_FreeTexturePages();
    }
    strcpy(g_LevelFileName, "");
    memset(g_TexturePageBuffer8, 0, sizeof(uint8_t *) * MAX_TEXTURE_PAGES);
    g_TextureInfoCount = 0;
}

BOOL __cdecl S_ReloadLevelGraphics(
    const bool reload_palettes, const bool reload_tex_pages)
{
    if (g_LevelFileName[0] != '\0') {
        VFILE *const file = VFile_CreateFromPath(g_LevelFileName);
        if (file == NULL) {
            return false;
        }

        if (reload_palettes && g_SavedAppSettings.render_mode == RM_SOFTWARE) {
            VFile_SetPos(file, g_LevelFilePalettesOffset);
            Level_LoadPalettes(file);

            VFile_SetPos(file, g_LevelFileDepthQOffset);
            Level_LoadDepthQ(file);
        }

        if (reload_tex_pages) {
            if (g_SavedAppSettings.render_mode == RM_HARDWARE) {
                HWR_FreeTexturePages();
            }
            VFile_SetPos(file, g_LevelFileTexPagesOffset);
            Level_LoadTexturePages(file);
        }

        VFile_Close(file);
    }

    if (reload_palettes) {
        Inv_InitColors();
    }

    return true;
}

int32_t __cdecl SE_ReadAppSettings(APP_SETTINGS *const settings)
{
    if (!OpenGameRegistryKey("System")) {
        return 0;
    }

    bool rc;
    GUID value;
    rc = GetRegistryGuidValue("PreferredDisplayAdapterGUID", &value, 0);
    settings->preferred_display_adapter =
        WinVidGetDisplayAdapter(rc ? &value : 0);

    settings->preferred_sound_adapter = NULL;
    settings->preferred_joystick = NULL;

    DISPLAY_MODE dm;
    GetRegistryDwordValue(
        "RenderMode", (DWORD *)&settings->render_mode, RM_HARDWARE);
    GetRegistryBoolValue("Dither", &settings->dither, false);
    GetRegistryBoolValue("ZBuffer", &settings->zbuffer, true);
    GetRegistryBoolValue(
        "BilinearFiltering", &settings->bilinear_filtering, true);
    GetRegistryBoolValue("TripleBuffering", &settings->triple_buffering, false);
    GetRegistryBoolValue("FullScreen", &settings->fullscreen, true);
    GetRegistryDwordValue(
        "WindowWidth", (DWORD *)&settings->window_width, 1280);
    GetRegistryDwordValue(
        "WindowHeight", (DWORD *)&settings->window_height, 720);
    GetRegistryDwordValue(
        "AspectMode", (DWORD *)&settings->aspect_mode, AM_4_3);
    GetRegistryDwordValue("FullScreenWidth", (DWORD *)&dm.width, 1280);
    GetRegistryDwordValue("FullScreenHeight", (DWORD *)&dm.height, 720);
    GetRegistryDwordValue("FullScreenBPP", (DWORD *)&dm.bpp, 16);
    GetRegistryBoolValue("SoundEnabled", &settings->sound_enabled, true);
    GetRegistryBoolValue("LaraMic", &settings->lara_mic, false);
    GetRegistryBoolValue("JoystickEnabled", &settings->joystick_enabled, true);
    GetRegistryBoolValue(
        "Disable16BitTextures", &settings->disable_16bit_textures, false);
    GetRegistryBoolValue(
        "DontSortPrimitives", &settings->dont_sort_primitives, false);
    GetRegistryDwordValue(
        "TexelAdjustMode", (DWORD *)&settings->texel_adjust_mode, TAM_ALWAYS);
    GetRegistryDwordValue(
        "NearestAdjustment", (DWORD *)&settings->nearest_adjustment, 16);
    GetRegistryDwordValue(
        "LinearAdjustment", (DWORD *)&settings->linear_adjustment, 128);
    GetRegistryBoolValue("FlipBroken", &settings->flip_broken, false);

    if (settings->render_mode != RM_HARDWARE
        && settings->render_mode != RM_SOFTWARE) {
        settings->render_mode = RM_SOFTWARE;
    }
    if (settings->aspect_mode != AM_ANY && settings->aspect_mode != AM_16_9) {
        settings->aspect_mode = AM_4_3;
    }
    if (settings->texel_adjust_mode != TAM_DISABLED
        && settings->texel_adjust_mode != TAM_BILINEAR_ONLY) {
        settings->texel_adjust_mode = TAM_ALWAYS;
    }
    CLAMP(settings->nearest_adjustment, 0, 256);
    CLAMP(settings->linear_adjustment, 0, 256);

    GetRegistryBoolValue(
        "PerspectiveCorrect", &settings->perspective_correct,
        settings->render_mode != RM_SOFTWARE);

    DISPLAY_MODE_LIST *disp_mode_list =
        &settings->preferred_display_adapter->body.hw_disp_mode_list;
    if (settings->render_mode == RM_SOFTWARE) {
        dm.bpp = 8;
        disp_mode_list =
            &settings->preferred_display_adapter->body.sw_disp_mode_list;
    }
    dm.vga = VGA_NO_VGA;

    const DISPLAY_MODE_NODE *head = disp_mode_list->head;
    while (head != NULL) {
        if (!CompareVideoModes(&head->body, &dm)) {
            break;
        }
        head = head->next;
    }
    settings->video_mode = head;

    CloseGameRegistryKey();
    return IsNewRegistryKeyCreated() ? 2 : 1;
}
