#include "decomp/decomp.h"
#include "game/hwr.h"
#include "global/funcs.h"
#include "global/vars.h"

int32_t __cdecl CreateTexturePage(
    const int32_t width, const int32_t height, const bool alpha)
{
    const int32_t palette_idx = GetFreeTexturePageIndex();
    if (palette_idx < 0) {
        return -1;
    }
    TEXPAGE_DESC *desc = &g_TexturePages[palette_idx];
    memset(desc, 0, sizeof(TEXPAGE_DESC));
    desc->status = 1;
    desc->width = width;
    desc->height = height;
    desc->palette = (LPDIRECTDRAWPALETTE)alpha;
    if (!CreateTexturePageSurface(desc)) {
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

bool __cdecl CreateTexturePageSurface(TEXPAGE_DESC *const desc)
{
    DDSURFACEDESC dsp = { 0 };
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    dsp.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    dsp.dwWidth = desc->width;
    dsp.dwHeight = desc->height;
    dsp.ddpfPixelFormat = g_TextureFormat.pixel_fmt;

    if (FAILED(DDrawSurfaceCreate(&dsp, &desc->sys_mem_surface))) {
        return false;
    }

    if (desc->palette != NULL
        && FAILED(desc->sys_mem_surface->lpVtbl->SetPalette(
            desc->sys_mem_surface, desc->palette))) {
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
    TEXPAGE_DESC *const desc = &g_TexturePages[page_idx];
    TexturePageReleaseVidMemSurface(desc);
    if (desc->sys_mem_surface != NULL) {
        desc->sys_mem_surface->lpVtbl->Release(desc->sys_mem_surface);
        desc->sys_mem_surface = NULL;
    }
    desc->status = 0;
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

    TEXPAGE_DESC *const desc = &g_TexturePages[page_idx];
    if (reset || desc->vid_mem_surface == NULL) {
        rc = SUCCEEDED(
            DDrawSurfaceRestoreLost(desc->vid_mem_surface, NULL, false));
    }

    if (!rc) {
        TexturePageReleaseVidMemSurface(desc);
        rc = TexturePageInit(desc);
    }

    if (!rc) {
        return false;
    }

    DDrawSurfaceRestoreLost(desc->sys_mem_surface, 0, 0);
    LPDIRECT3DTEXTURE2 sys_mem_texture = Create3DTexture(desc->sys_mem_surface);
    if (sys_mem_texture == NULL) {
        return false;
    }

    rc = SUCCEEDED(
        desc->texture_3d->lpVtbl->Load(desc->texture_3d, sys_mem_texture));
    sys_mem_texture->lpVtbl->Release(sys_mem_texture);
    return rc;
}
