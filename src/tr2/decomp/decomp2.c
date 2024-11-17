#include "decomp/decomp.h"
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
