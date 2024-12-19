#pragma once

#include "../common.h"
#include "../config.h"
#include "../gl/buffer.h"
#include "../gl/program.h"
#include "../gl/texture.h"
#include "../gl/vertex_array.h"
#include "2d_surface.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GFX_2D_EFFECT_NONE = 0,
    GFX_2D_EFFECT_VIGNETTE = 1,
} GFX_2D_EFFECT;

typedef struct GFX_2D_RENDERER GFX_2D_RENDERER;

GFX_2D_RENDERER *GFX_2D_Renderer_Create(void);
void GFX_2D_Renderer_Destroy(GFX_2D_RENDERER *renderer);

void GFX_2D_Renderer_UploadSurface(
    GFX_2D_RENDERER *renderer, GFX_2D_SURFACE *surface);
void GFX_2D_Renderer_UploadAlphaSurface(
    GFX_2D_RENDERER *renderer, GFX_2D_SURFACE *surface);
void GFX_2D_Renderer_Upload(
    GFX_2D_RENDERER *renderer, GFX_2D_SURFACE_DESC *desc, const uint8_t *data);

void GFX_2D_Renderer_SetPalette(
    GFX_2D_RENDERER *renderer, const GFX_COLOR *palette);
void GFX_2D_Renderer_SetRepeat(GFX_2D_RENDERER *renderer, int32_t x, int32_t y);
void GFX_2D_Renderer_SetEffect(GFX_2D_RENDERER *renderer, GFX_2D_EFFECT filter);
void GFX_2D_Renderer_SetTint(GFX_2D_RENDERER *renderer, GFX_COLOR color);

void GFX_2D_Renderer_Render(GFX_2D_RENDERER *renderer);
