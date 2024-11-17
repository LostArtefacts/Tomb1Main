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
