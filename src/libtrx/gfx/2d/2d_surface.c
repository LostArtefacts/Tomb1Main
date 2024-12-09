#include "gfx/2d/2d_surface.h"

#include "debug.h"
#include "gfx/context.h"
#include "log.h"
#include "memory.h"

#include <string.h>

GFX_2D_SURFACE *GFX_2D_Surface_Create(const GFX_2D_SURFACE_DESC *const desc)
{
    GFX_2D_SURFACE *surface = Memory_Alloc(sizeof(GFX_2D_SURFACE));
    GFX_2D_Surface_Init(surface, desc);
    return surface;
}

GFX_2D_SURFACE *GFX_2D_Surface_CreateFromImage(const IMAGE *const image)
{
    GFX_2D_SURFACE *surface = Memory_Alloc(sizeof(GFX_2D_SURFACE));
    surface->desc.width = image->width;
    surface->desc.height = image->height;
    surface->desc.bit_count = 24;
    surface->desc.tex_format = GL_RGB;
    surface->desc.tex_type = GL_UNSIGNED_BYTE;
    surface->desc.pitch = surface->desc.width * (surface->desc.bit_count / 8);
    surface->buffer = Memory_Alloc(surface->desc.pitch * surface->desc.height);
    memcpy(
        surface->buffer, image->data,
        surface->desc.pitch * surface->desc.height);
    return surface;
}

void GFX_2D_Surface_Free(GFX_2D_SURFACE *const surface)
{
    if (surface) {
        GFX_2D_Surface_Close(surface);
        Memory_Free(surface);
    }
}

void GFX_2D_Surface_Init(
    GFX_2D_SURFACE *const surface, const GFX_2D_SURFACE_DESC *const desc)
{
    surface->desc = *desc;

    GFX_2D_SURFACE_DESC display_desc = {
        .bit_count = 32,
        .width = GFX_Context_GetDisplayWidth(),
        .height = GFX_Context_GetDisplayHeight(),
    };

    if (!surface->desc.width || !surface->desc.height) {
        surface->desc.width = display_desc.width;
        surface->desc.height = display_desc.height;
    }

    if (!surface->desc.bit_count) {
        surface->desc.bit_count = display_desc.bit_count;
    }

    if (!surface->desc.tex_format) {
        surface->desc.tex_format = GL_BGRA;
    }
    if (!surface->desc.tex_type) {
        surface->desc.tex_type = GL_UNSIGNED_INT_8_8_8_8_REV;
    }

    surface->desc.pitch = surface->desc.width * (surface->desc.bit_count / 8);

    surface->buffer = Memory_Alloc(surface->desc.pitch * surface->desc.height);
}

void GFX_2D_Surface_Close(GFX_2D_SURFACE *const surface)
{
    Memory_FreePointer(&surface->buffer);
}

void GFX_2D_Surface_Clear(GFX_2D_SURFACE *const surface, const uint8_t value)
{
    memset(surface->buffer, value, surface->desc.pitch * surface->desc.height);
}
