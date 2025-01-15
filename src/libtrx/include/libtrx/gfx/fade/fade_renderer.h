#pragma once

#include "../config.h"
#include "../gl/buffer.h"
#include "../gl/program.h"
#include "../gl/texture.h"
#include "../gl/vertex_array.h"

#include <stdint.h>

typedef struct GFX_FADE_RENDERER GFX_FADE_RENDERER;

GFX_FADE_RENDERER *GFX_FadeRenderer_Create(void);
void GFX_FadeRenderer_Destroy(GFX_FADE_RENDERER *renderer);

void GFX_FadeRenderer_SetOpacity(GFX_FADE_RENDERER *renderer, float opacity);
void GFX_FadeRenderer_Render(GFX_FADE_RENDERER *renderer);
