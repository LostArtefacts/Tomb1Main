#include "game/const.h"
#include "game/game_buf.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "game/shell.h"

static int32_t m_ObjectTextureCount = 0;
static OBJECT_TEXTURE *m_ObjectTextures = NULL;
static SPRITE_TEXTURE *m_SpriteTextures = NULL;
static ANIMATED_TEXTURE_RANGE *m_AnimTextureRanges = NULL;

void Output_InitialiseObjectTextures(const int32_t num_textures)
{
    if (num_textures > MAX_OBJECT_TEXTURES) {
        Shell_ExitSystemFmt(
            "Too many object textures: %d (max=%d)", num_textures,
            MAX_OBJECT_TEXTURES);
        return;
    }

    m_ObjectTextureCount = num_textures;
    m_ObjectTextures = num_textures == 0
        ? NULL
        : GameBuf_Alloc(
              sizeof(OBJECT_TEXTURE) * num_textures, GBUF_OBJECT_TEXTURES);
}

void Output_InitialiseSpriteTextures(const int32_t num_textures)
{
    if (num_textures > MAX_SPRITE_TEXTURES) {
        Shell_ExitSystemFmt(
            "Too many sprite textures: %d (max=%d)", num_textures,
            MAX_SPRITE_TEXTURES);
        return;
    }

    m_SpriteTextures = num_textures == 0
        ? NULL
        : GameBuf_Alloc(
              sizeof(SPRITE_TEXTURE) * num_textures, GBUF_SPRITE_TEXTURES);
}

void Output_InitialiseAnimatedTextures(const int32_t num_ranges)
{
    m_AnimTextureRanges = num_ranges == 0
        ? NULL
        : GameBuf_Alloc(
              sizeof(ANIMATED_TEXTURE_RANGE) * num_ranges,
              GBUF_ANIMATED_TEXTURE_RANGES);
}

int32_t Output_GetObjectTextureCount(void)
{
    return m_ObjectTextureCount;
}

OBJECT_TEXTURE *Output_GetObjectTexture(const int32_t texture_idx)
{
    return &m_ObjectTextures[texture_idx];
}

SPRITE_TEXTURE *Output_GetSpriteTexture(const int32_t texture_idx)
{
    return &m_SpriteTextures[texture_idx];
}

ANIMATED_TEXTURE_RANGE *Output_GetAnimatedTextureRange(const int32_t range_idx)
{
    return &m_AnimTextureRanges[range_idx];
}

void Output_CycleAnimatedTextures(void)
{
    const ANIMATED_TEXTURE_RANGE *range = m_AnimTextureRanges;
    for (; range != NULL; range = range->next_range) {
        int32_t i = 0;
        const OBJECT_TEXTURE temp = m_ObjectTextures[range->textures[i]];
        for (; i < range->num_textures - 1; i++) {
            m_ObjectTextures[range->textures[i]] =
                m_ObjectTextures[range->textures[i + 1]];
        }
        m_ObjectTextures[range->textures[i]] = temp;
    }

    for (int32_t i = 0; i < MAX_STATIC_OBJECTS; i++) {
        const STATIC_OBJECT_2D *const object = Object_GetStaticObject2D(i);
        if (!object->loaded || object->frame_count == 1) {
            continue;
        }

        const int16_t frame_count = object->frame_count;
        const SPRITE_TEXTURE temp = m_SpriteTextures[object->texture_idx];
        for (int32_t j = 0; j < frame_count - 1; j++) {
            m_SpriteTextures[object->texture_idx + j] =
                m_SpriteTextures[object->texture_idx + j + 1];
        }
        m_SpriteTextures[object->texture_idx + frame_count - 1] = temp;
    }
}
