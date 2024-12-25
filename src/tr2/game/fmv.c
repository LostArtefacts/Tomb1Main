#include "game/fmv.h"

#include "config.h"
#include "game/input.h"
#include "game/music.h"
#include "game/render/common.h"
#include "game/shell.h"
#include "game/sound.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/engine/video.h>
#include <libtrx/filesystem.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

#include <string.h>

static bool m_Muted = false;

static void M_Play(const char *file_name);

static void *M_AllocateSurface(int32_t width, int32_t height, void *user_data);
static void M_DeallocateSurface(void *surface, void *user_data);
static void M_ClearSurface(void *surface, void *user_data);
static void M_RenderBegin(void *surface, void *user_data);
static void M_RenderEnd(void *surface, void *user_data);
static void *M_LockSurface(void *surface, void *user_data);
static void M_UnlockSurface(void *surface, void *user_data);
static void M_UploadSurface(void *surface, void *user_data);

static void *M_AllocateSurface(
    const int32_t width, const int32_t height, void *const user_data)
{
    GFX_2D_SURFACE_DESC surface_desc = {
        .width = width,
        .height = height,
        .tex_format =
            g_Config.rendering.render_mode == RM_SOFTWARE ? GL_RED : GL_BGRA,
        .tex_type = g_Config.rendering.render_mode == RM_SOFTWARE
            ? GL_UNSIGNED_BYTE
            : GL_UNSIGNED_INT_8_8_8_8_REV,
    };
    return GFX_2D_Surface_Create(&surface_desc);
}

static void M_DeallocateSurface(void *const surface, void *const user_data)
{
    GFX_2D_Surface_Free(surface);
}

static void M_ClearSurface(void *const surface, void *const user_data)
{
    ASSERT(surface != NULL);
    GFX_2D_SURFACE *const surface_ = surface;
    memset(surface_->buffer, 0, surface_->desc.pitch * surface_->desc.height);
}

static void M_RenderBegin(void *const surface, void *const user_data)
{
    GFX_Context_Clear();
}

static void M_RenderEnd(void *const surface, void *const user_data)
{
    GFX_Context_SwapBuffers();
}

static void *M_LockSurface(void *const surface, void *const user_data)
{
    ASSERT(surface != NULL);
    GFX_2D_SURFACE *const surface_ = surface;
    return surface_->buffer;
}

static void M_UnlockSurface(void *const surface, void *const user_data)
{
}

static void M_UploadSurface(void *const surface, void *const user_data)
{
    GFX_2D_RENDERER *renderer_2d = user_data;
    GFX_2D_SURFACE *surface_ = surface;
    GFX_2D_Renderer_Upload(renderer_2d, &surface_->desc, surface_->buffer);
    GFX_2D_Renderer_Render(renderer_2d);
}

static bool M_EnterFMVMode(void)
{
    Music_Stop();
    return true;
}

static void M_ExitFMVMode(void)
{
    if (!g_IsGameToExit) {
        Render_Reset(RENDER_RESET_ALL);
    }
}

static void M_Play(const char *const file_name)
{
    const char *full_path = File_GetFullPath(file_name);

    VIDEO *video = Video_Open(file_name);
    if (video == NULL) {
        return;
    }

    g_IsFMVPlaying = true;
    GFX_2D_RENDERER *const renderer_2d = GFX_2D_Renderer_Create();

    // Populate the palette with a palette corresponding to
    // AV_PIX_FMT_RGB8
    GFX_COLOR palette[256];
    for (int32_t i = 0; i < 256; i++) {
        GFX_COLOR *const col = &palette[i];
        col->r = 0x24 * (i >> 5);
        col->g = 0x24 * ((i >> 2) & 7);
        col->b = 0x55 * (i & 3);
    }

    Video_SetSurfaceAllocatorFunc(video, M_AllocateSurface, NULL);
    Video_SetSurfaceDeallocatorFunc(video, M_DeallocateSurface, NULL);
    Video_SetSurfaceClearFunc(video, M_ClearSurface, NULL);
    Video_SetRenderBeginFunc(video, M_RenderBegin, NULL);
    Video_SetRenderEndFunc(video, M_RenderEnd, NULL);
    Video_SetSurfaceLockFunc(video, M_LockSurface, NULL);
    Video_SetSurfaceUnlockFunc(video, M_UnlockSurface, NULL);
    Video_SetSurfaceUploadFunc(video, M_UploadSurface, renderer_2d);

    Video_Start(video);
    while (video->is_playing) {
        Video_SetVolume(
            video,
            m_Muted
                ? 0
                : g_Config.audio.sound_volume / (float)Sound_GetMaxVolume());

        Video_SetSurfaceSize(
            video, Shell_GetCurrentDisplayWidth(),
            Shell_GetCurrentDisplayHeight());
        if (g_Config.rendering.render_mode == RM_SOFTWARE) {
            Video_SetSurfacePixelFormat(video, AV_PIX_FMT_RGB8);
            GFX_2D_Renderer_SetPalette(renderer_2d, palette);
        } else {
            Video_SetSurfacePixelFormat(video, AV_PIX_FMT_BGRA);
            GFX_2D_Renderer_SetPalette(renderer_2d, NULL);
        }

        Video_PumpEvents(video);
        Shell_ProcessEvents();

        Input_Update();
        Shell_ProcessInput();
        if (g_InputDB.menu_back || g_InputDB.menu_confirm || g_IsGameToExit) {
            Video_Stop(video);
            break;
        }
    }
    Video_Close(video);

    GFX_2D_Renderer_Destroy(renderer_2d);
    Memory_FreePointer(&full_path);
    g_IsFMVPlaying = false;
}

bool FMV_Play(const char *const file_name)
{
    if (M_EnterFMVMode()) {
        M_Play(file_name);
    }
    M_ExitFMVMode();
    return g_IsGameToExit;
}

bool FMV_PlayIntro(const char *const file_name_1, const char *const file_name_2)
{
    if (M_EnterFMVMode()) {
        M_Play(file_name_1);
        M_Play(file_name_2);
    }
    M_ExitFMVMode();
    return g_IsGameToExit;
}

bool FMV_IsPlaying(void)
{
    return g_IsFMVPlaying;
}
