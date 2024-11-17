#include "decomp/decomp.h"
#include "game/hwr.h"
#include "global/funcs.h"
#include "global/vars.h"

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
