#pragma once

#include "./types.h"

void Output_InitialiseObjectTextures(int32_t num_textures);
void Output_InitialiseSpriteTextures(int32_t num_textures);
void Output_InitialiseAnimatedTextures(int32_t num_ranges);

int32_t Output_GetObjectTextureCount(void);
OBJECT_TEXTURE *Output_GetObjectTexture(int32_t texture_idx);
SPRITE_TEXTURE *Output_GetSpriteTexture(int32_t texture_idx);
ANIMATED_TEXTURE_RANGE *Output_GetAnimatedTextureRange(int32_t range_idx);

void Output_CycleAnimatedTextures(void);
