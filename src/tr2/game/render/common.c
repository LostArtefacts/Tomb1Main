#include "game/render/common.h"

#include "config.h"
#include "decomp/decomp.h"
#include "game/render/hwr.h"
#include "game/render/priv.h"
#include "game/render/swr.h"
#include "game/render/util.h"
#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/gfx/fade/fade_renderer.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>

static RENDERER m_Renderer_SW = { 0 };
static RENDERER m_Renderer_HW = { 0 };
static RENDERER *m_PreviousRenderer = NULL;
static GFX_FADE_RENDERER *m_FadeRenderer = NULL;
static GFX_2D_RENDERER *m_BackgroundRenderer = NULL;

static struct {
    bool ready;
    GFX_2D_SURFACE *surface;
    const PHD_TEXTURE *texture;
    int32_t repeat_x;
    int32_t repeat_y;
} m_Background = { 0 };

static RENDERER *M_GetRenderer(void);
static void M_ReuploadBackground(void);
static void M_ResetPolyList(void);

static RENDERER *M_GetRenderer(void)
{
    RENDERER *r = NULL;
    if (g_Config.rendering.render_mode == RM_SOFTWARE) {
        r = &m_Renderer_SW;
    } else if (g_Config.rendering.render_mode == RM_HARDWARE) {
        r = &m_Renderer_HW;
    }
    ASSERT(r != NULL);
    return r;
}

static void M_ReuploadBackground(void)
{
    // Toggling the bilinear filter causes the texture UVs to be readjusted
    // depending on the player's settings. Because the texture UVs are cached
    // on the GPU by Render_LoadBackgroundFromTexture, they need to be updated
    // whenever the UVs readjust.
    if (m_Background.texture != NULL) {
        Render_LoadBackgroundFromTexture(
            m_Background.texture, m_Background.repeat_x, m_Background.repeat_y);
    }
}

static void M_ResetPolyList(void)
{
    g_SurfaceCount = 0;
    g_Sort3DPtr = g_SortBuffer;
    g_Info3DPtr = g_Info3DBuffer;

    RENDERER *const r = M_GetRenderer();
    if (r->ResetPolyList != NULL) {
        r->ResetPolyList(r);
    }
}

void Render_Init(void)
{
    LOG_DEBUG("");
    GFX_Context_Attach(g_SDLWindow, GFX_GL_33C);
    GFX_Context_SetRenderingMode(GFX_RM_FRAMEBUFFER);
    m_FadeRenderer = GFX_FadeRenderer_Create();
    m_BackgroundRenderer = GFX_2D_Renderer_Create();
    Renderer_SW_Prepare(&m_Renderer_SW);
    Renderer_HW_Prepare(&m_Renderer_HW);
}

void Render_Shutdown(void)
{
    LOG_DEBUG("");
    RENDERER *const r = M_GetRenderer();
    if (r != NULL) {
        r->Close(r);
        r->Shutdown(r);
    }

    if (m_Background.surface != NULL) {
        GFX_2D_Surface_Free(m_Background.surface);
        m_Background.surface = NULL;
    }

    if (m_FadeRenderer != NULL) {
        GFX_FadeRenderer_Destroy(m_FadeRenderer);
        m_FadeRenderer = NULL;
    }

    if (m_BackgroundRenderer != NULL) {
        GFX_2D_Renderer_Destroy(m_BackgroundRenderer);
        m_BackgroundRenderer = NULL;
    }

    GFX_Context_Detach();
}

void Render_Reset(const RENDER_RESET_FLAGS reset_flags)
{
    LOG_DEBUG("reset_flags=%x", reset_flags);

    if (m_PreviousRenderer != NULL) {
        m_PreviousRenderer->Close(m_PreviousRenderer);
    }

    RENDERER *const r = M_GetRenderer();
    if (!r->initialized) {
        r->Init(r);
    }
    r->Open(r);
    m_PreviousRenderer = r;

    r->Reset(r, reset_flags);

    if (reset_flags & (RENDER_RESET_PARAMS | RENDER_RESET_TEXTURES)) {
        Render_AdjustTextureUVs(reset_flags & RENDER_RESET_TEXTURES);
        M_ReuploadBackground();
    }

    if (reset_flags & RENDER_RESET_PARAMS) {
        GFX_Context_SetWireframeMode(g_Config.rendering.enable_wireframe);
        GFX_Context_SetLineWidth(g_Config.rendering.wireframe_width);
    }
    LOG_DEBUG("reset finished");
}

void Render_SetupDisplay(
    const int32_t window_border, const int32_t window_width,
    const int32_t window_height, const int32_t screen_width,
    const int32_t screen_height)
{
    LOG_DEBUG("%dx%d", screen_width, screen_height);
    GFX_Context_SetWindowBorder(window_border);
    GFX_Context_SetWindowSize(window_width, window_height);
    GFX_Context_SetDisplaySize(screen_width, screen_height);
    Render_Reset(RENDER_RESET_VIEWPORT);
}

void Render_BeginScene(void)
{
    GFX_Context_Clear();
    RENDERER *const r = M_GetRenderer();
    r->BeginScene(r);
    M_ResetPolyList();
}

void Render_EndScene(void)
{
    RENDERER *const r = M_GetRenderer();
    r->EndScene(r);
    GFX_Context_SwapBuffers();
}

void Render_LoadBackgroundFromTexture(
    const PHD_TEXTURE *const texture, const int32_t repeat_x,
    const int32_t repeat_y)
{
    if (g_TexturePageBuffer16[texture->tex_page] == NULL) {
        return;
    }

    if (m_Background.surface != NULL) {
        GFX_2D_Surface_Free(m_Background.surface);
        m_Background.surface = NULL;
    }

    m_Background.ready = true;
    m_Background.texture = texture;
    m_Background.repeat_x = repeat_x;
    m_Background.repeat_y = repeat_y;

    GFX_2D_SURFACE_DESC desc = {
        .width = TEXTURE_PAGE_WIDTH,
        .height = TEXTURE_PAGE_HEIGHT,
        .bit_count = 16,
        .tex_format = GL_BGRA,
        .tex_type = GL_UNSIGNED_SHORT_1_5_5_5_REV,
        .uv = {
            {
                texture->uv[0].u / 256.0f / TEXTURE_PAGE_WIDTH,
                texture->uv[0].v / 256.0f / TEXTURE_PAGE_HEIGHT},
            {
                texture->uv[1].u / 256.0f / TEXTURE_PAGE_WIDTH,
                texture->uv[1].v / 256.0f / TEXTURE_PAGE_HEIGHT},
            {
                texture->uv[2].u / 256.0f / TEXTURE_PAGE_WIDTH,
                texture->uv[2].v / 256.0f / TEXTURE_PAGE_HEIGHT},
            {
                texture->uv[3].u / 256.0f / TEXTURE_PAGE_WIDTH,
                texture->uv[3].v / 256.0f / TEXTURE_PAGE_HEIGHT},
        },
        .pitch = TEXTURE_PAGE_WIDTH * 2,
    };
    GFX_2D_Renderer_Upload(
        m_BackgroundRenderer, &desc,
        (uint8_t *)g_TexturePageBuffer16[texture->tex_page]);
    GFX_2D_Renderer_SetRepeat(m_BackgroundRenderer, repeat_x, repeat_y);
    GFX_2D_Renderer_SetEffect(m_BackgroundRenderer, GFX_2D_EFFECT_VIGNETTE);
}

void Render_LoadBackgroundFromImage(const IMAGE *const image)
{
    if (m_Background.surface != NULL) {
        GFX_2D_Surface_Free(m_Background.surface);
        m_Background.surface = NULL;
    }
    ASSERT(image != NULL);
    m_Background.ready = true;
    m_Background.surface = GFX_2D_Surface_CreateFromImage(image);
    m_Background.texture = NULL;
    m_Background.repeat_x = 1;
    m_Background.repeat_y = 1;
    GFX_2D_Renderer_UploadSurface(m_BackgroundRenderer, m_Background.surface);
    GFX_2D_Renderer_SetRepeat(m_BackgroundRenderer, 1, 1);
    GFX_2D_Renderer_SetEffect(m_BackgroundRenderer, GFX_2D_EFFECT_NONE);
}

void Render_UnloadBackground(void)
{
    if (m_Background.surface != NULL) {
        GFX_2D_Surface_Free(m_Background.surface);
        m_Background.surface = NULL;
    }
    m_Background.texture = NULL;
    m_Background.ready = false;
    GFX_2D_Renderer_SetRepeat(m_BackgroundRenderer, 1, 1);
    GFX_2D_Renderer_SetEffect(m_BackgroundRenderer, GFX_2D_EFFECT_NONE);
}

void Render_DrawBackground(void)
{
    if (!m_Background.ready) {
        return;
    }
    GFX_2D_Renderer_Render(m_BackgroundRenderer);
    Render_EnableZBuffer(true, true);
}

void Render_ClearZBuffer(void)
{
    RENDERER *const r = M_GetRenderer();
    if (r->ClearZBuffer != NULL) {
        r->ClearZBuffer(r);
    }
}

void Render_EnableZBuffer(const bool z_write_enable, const bool z_test_enable)
{
    RENDERER *const r = M_GetRenderer();
    if (r->EnableZBuffer != NULL) {
        r->EnableZBuffer(r, z_write_enable, z_test_enable);
    }
}

void Render_DrawPolyList(void)
{
    RENDERER *const r = M_GetRenderer();
    r->DrawPolyList(r);
    M_ResetPolyList();
}

void Render_DrawBlackRectangle(const int32_t opacity)
{
    GFX_FadeRenderer_SetOpacity(m_FadeRenderer, opacity / 255.0f);
    GFX_FadeRenderer_Render(m_FadeRenderer);
}

const int16_t *Render_InsertObjectG3(
    const int16_t *const obj_ptr, const int32_t num, const SORT_TYPE sort_type)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertObjectG3 != NULL) {
        return r->InsertObjectG3(r, obj_ptr, num, sort_type);
    }
    return obj_ptr + num * 4;
}

const int16_t *Render_InsertObjectG4(
    const int16_t *const obj_ptr, const int32_t num, const SORT_TYPE sort_type)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertObjectG4 != NULL) {
        return r->InsertObjectG4(r, obj_ptr, num, sort_type);
    }
    return obj_ptr + num * 5;
}

const int16_t *Render_InsertObjectGT3(
    const int16_t *const obj_ptr, const int32_t num, const SORT_TYPE sort_type)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertObjectGT3 != NULL) {
        return r->InsertObjectGT3(r, obj_ptr, num, sort_type);
    }
    return obj_ptr + num * 4;
}

const int16_t *Render_InsertObjectGT4(
    const int16_t *const obj_ptr, const int32_t num, const SORT_TYPE sort_type)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertObjectGT4 != NULL) {
        return r->InsertObjectGT4(r, obj_ptr, num, sort_type);
    }
    return obj_ptr + num * 5;
}

void Render_InsertLine(
    const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1,
    const int32_t z, const uint8_t color_idx)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertLine != NULL) {
        r->InsertLine(r, x0, y0, x1, y1, z, color_idx);
    }
}

void Render_InsertFlatRect(
    const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1,
    const int32_t z, const uint8_t color_idx)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertFlatRect != NULL) {
        r->InsertFlatRect(r, x0, y0, x1, y1, z, color_idx);
    }
}

void Render_InsertTransQuad(
    const int32_t x, const int32_t y, const int32_t width, const int32_t height,
    const int32_t z)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertTransQuad != NULL) {
        r->InsertTransQuad(r, x, y, width, height, z);
    }
}

void Render_InsertTransOctagon(const PHD_VBUF *const vbuf, const int16_t shade)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertTransOctagon != NULL) {
        r->InsertTransOctagon(r, vbuf, shade);
    }
}

void Render_InsertSprite(
    const int32_t z, const int32_t x0, const int32_t y0, const int32_t x1,
    const int32_t y1, const int32_t sprite_idx, const int16_t shade)
{
    RENDERER *const r = M_GetRenderer();
    if (r->InsertSprite != NULL) {
        r->InsertSprite(r, z, x0, y0, x1, y1, sprite_idx, shade);
    }
}

void Render_SetWet(const bool is_wet)
{
    RENDERER *const r = M_GetRenderer();
    if (r->SetWet != NULL) {
        r->SetWet(r, is_wet);
    }
}
