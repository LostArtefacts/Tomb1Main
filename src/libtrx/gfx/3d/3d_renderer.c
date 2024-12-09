#include "gfx/3d/3d_renderer.h"

#include "debug.h"
#include "gfx/context.h"
#include "gfx/gl/utils.h"
#include "log.h"
#include "memory.h"

#include <stddef.h>

struct GFX_3D_RENDERER {
    const GFX_CONFIG *config;

    GFX_GL_PROGRAM program;
    GFX_GL_SAMPLER sampler;
    GFX_3D_VERTEX_STREAM vertex_stream;

    GFX_GL_TEXTURE *textures[GFX_MAX_TEXTURES];
    GFX_GL_TEXTURE *env_map_texture;
    int selected_texture_num;
    GFX_BLEND_MODE selected_blend_mode;

    // shader variable locations
    GLint loc_mat_projection;
    GLint loc_mat_model_view;
    GLint loc_texturing_enabled;
    GLint loc_smoothing_enabled;
    GLint loc_color_key_enabled;
};

static void M_SelectTextureImpl(GFX_3D_RENDERER *renderer, int texture_num);

static void M_SelectTextureImpl(
    GFX_3D_RENDERER *const renderer, const int texture_num)
{
    ASSERT(renderer != NULL);

    GFX_GL_TEXTURE *texture = NULL;
    if (texture_num == GFX_ENV_MAP_TEXTURE) {
        texture = renderer->env_map_texture;
    } else if (texture_num != GFX_NO_TEXTURE) {
        ASSERT(texture_num >= 0);
        ASSERT(texture_num < GFX_MAX_TEXTURES);
        texture = renderer->textures[texture_num];
    }

    if (texture == NULL) {
        glBindTexture(GL_TEXTURE_2D, 0);
        GFX_GL_CheckError();
        return;
    }

    GFX_GL_Texture_Bind(texture);
}

static void M_RestoreTexture(GFX_3D_RENDERER *renderer)
{
    ASSERT(renderer != NULL);
    M_SelectTextureImpl(renderer, renderer->selected_texture_num);
}

GFX_3D_RENDERER *GFX_3D_Renderer_Create(void)
{
    LOG_INFO("");
    GFX_3D_RENDERER *const renderer = Memory_Alloc(sizeof(GFX_3D_RENDERER));
    renderer->config = GFX_Context_GetConfig();

    renderer->selected_texture_num = GFX_NO_TEXTURE;
    for (int i = 0; i < GFX_MAX_TEXTURES; i++) {
        renderer->textures[i] = NULL;
    }

    GFX_GL_Sampler_Init(&renderer->sampler);
    GFX_GL_Sampler_Bind(&renderer->sampler, 0);
    GFX_GL_Sampler_Parameterf(
        &renderer->sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
    GFX_GL_Sampler_Parameteri(
        &renderer->sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GFX_GL_Sampler_Parameteri(
        &renderer->sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GFX_GL_Program_Init(&renderer->program);
    GFX_GL_Program_AttachShader(
        &renderer->program, GL_VERTEX_SHADER, "shaders/3d.glsl",
        renderer->config->backend);
    GFX_GL_Program_AttachShader(
        &renderer->program, GL_FRAGMENT_SHADER, "shaders/3d.glsl",
        renderer->config->backend);
    GFX_GL_Program_Link(&renderer->program);

    renderer->loc_mat_projection =
        GFX_GL_Program_UniformLocation(&renderer->program, "matProjection");
    renderer->loc_mat_model_view =
        GFX_GL_Program_UniformLocation(&renderer->program, "matModelView");
    renderer->loc_texturing_enabled =
        GFX_GL_Program_UniformLocation(&renderer->program, "texturingEnabled");
    renderer->loc_smoothing_enabled =
        GFX_GL_Program_UniformLocation(&renderer->program, "smoothingEnabled");
    renderer->loc_color_key_enabled =
        GFX_GL_Program_UniformLocation(&renderer->program, "colorKeyEnabled");

    GFX_GL_Program_FragmentData(&renderer->program, "fragColor");
    GFX_GL_Program_Bind(&renderer->program);

    // negate Z axis so the model is rendered behind the viewport, which is
    // better than having a negative z_near in the ortho matrix, which seems
    // to mess up depth testing
    GLfloat model_view[4][4] = {
        { +1.0f, +0.0f, +0.0f, +0.0f },
        { +0.0f, +1.0f, +0.0f, +0.0f },
        { +0.0f, +0.0f, -1.0f, +0.0f },
        { +0.0f, +0.0f, +0.0f, +1.0f },
    };
    GFX_GL_Program_UniformMatrix4fv(
        &renderer->program, renderer->loc_mat_model_view, 1, GL_FALSE,
        &model_view[0][0]);

    GFX_3D_VertexStream_Init(&renderer->vertex_stream);
    GFX_GL_CheckError();

    return renderer;
}

void GFX_3D_Renderer_Destroy(GFX_3D_RENDERER *const renderer)
{
    LOG_INFO("");
    ASSERT(renderer != NULL);

    GFX_3D_VertexStream_Close(&renderer->vertex_stream);
    GFX_GL_Program_Close(&renderer->program);
    GFX_GL_Sampler_Close(&renderer->sampler);
    Memory_Free(renderer);
}

void GFX_3D_Renderer_RenderBegin(GFX_3D_RENDERER *const renderer)
{
    ASSERT(renderer != NULL);
    glEnable(GL_BLEND);

    glLineWidth(renderer->config->line_width);
    glPolygonMode(
        GL_FRONT_AND_BACK,
        renderer->config->enable_wireframe ? GL_LINE : GL_FILL);
    GFX_GL_CheckError();

    renderer->vertex_stream.rendered_count = 0;

    GFX_GL_Program_Bind(&renderer->program);
    GFX_3D_VertexStream_Bind(&renderer->vertex_stream);
    GFX_GL_Sampler_Bind(&renderer->sampler, 0);

    M_RestoreTexture(renderer);

    const float left = 0.0f;
    const float top = 0.0f;
    const float right = GFX_Context_GetDisplayWidth();
    const float bottom = GFX_Context_GetDisplayHeight();
    const float z_near = -1e6;
    const float z_far = 1e6;
    GLfloat projection[4][4] = {
        { 2.0f / (right - left), 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f },
        { 0.0f, 0.0f, -2.0f / (z_far - z_near), 0.0f },
        { -(right + left) / (right - left), -(top + bottom) / (top - bottom),
          -(z_far + z_near) / (z_far - z_near), 1.0f }
    };

    GFX_GL_Program_UniformMatrix4fv(
        &renderer->program, renderer->loc_mat_projection, 1, GL_FALSE,
        &projection[0][0]);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    GFX_GL_CheckError();
}

void GFX_3D_Renderer_RenderEnd(GFX_3D_RENDERER *const renderer)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);

    GFX_GL_CheckError();
}

void GFX_3D_Renderer_ClearDepth(GFX_3D_RENDERER *const renderer)
{
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    glClear(GL_DEPTH_BUFFER_BIT);
    GFX_GL_CheckError();
}

int GFX_3D_Renderer_RegisterEnvironmentMap(GFX_3D_RENDERER *const renderer)
{
    ASSERT(renderer != NULL);
    ASSERT(renderer->env_map_texture == NULL);

    GFX_GL_TEXTURE *const texture = GFX_GL_Texture_Create(GL_TEXTURE_2D);
    renderer->env_map_texture = texture;

    M_RestoreTexture(renderer);
    GFX_GL_CheckError();
    return GFX_ENV_MAP_TEXTURE;
}

bool GFX_3D_Renderer_UnregisterEnvironmentMap(
    GFX_3D_RENDERER *const renderer, const int texture_num)
{
    ASSERT(renderer != NULL);

    GFX_GL_TEXTURE *const texture = renderer->env_map_texture;
    if (texture == NULL) {
        LOG_ERROR("No environment map registered");
        return false;
    }

    if (texture_num != GFX_ENV_MAP_TEXTURE) {
        LOG_ERROR("Invalid environment map texture ID");
        return false;
    }

    // unbind texture if currently bound
    if (renderer->selected_texture_num == texture_num) {
        M_SelectTextureImpl(renderer, GFX_NO_TEXTURE);
        renderer->selected_texture_num = GFX_NO_TEXTURE;
    }

    GFX_GL_Texture_Free(texture);
    renderer->env_map_texture = NULL;
    return true;
}

void GFX_3D_Renderer_FillEnvironmentMap(GFX_3D_RENDERER *const renderer)
{
    ASSERT(renderer != NULL);

    GFX_GL_TEXTURE *const env_map = renderer->env_map_texture;
    if (env_map != NULL) {
        GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
        GFX_GL_Texture_LoadFromBackBuffer(env_map);
        M_RestoreTexture(renderer);
    }
}

int GFX_3D_Renderer_RegisterTexturePage(
    GFX_3D_RENDERER *const renderer, const void *const data, const int width,
    const int height)
{
    ASSERT(renderer != NULL);
    ASSERT(data != NULL);
    GFX_GL_TEXTURE *const texture = GFX_GL_Texture_Create(GL_TEXTURE_2D);
    GFX_GL_Texture_Load(texture, data, width, height, GL_RGBA, GL_RGBA);

    int texture_num = GFX_NO_TEXTURE;
    for (int i = 0; i < GFX_MAX_TEXTURES; i++) {
        if (!renderer->textures[i]) {
            renderer->textures[i] = texture;
            texture_num = i;
            break;
        }
    }

    M_RestoreTexture(renderer);

    GFX_GL_CheckError();
    return texture_num;
}

bool GFX_3D_Renderer_UnregisterTexturePage(
    GFX_3D_RENDERER *const renderer, int texture_num)
{
    ASSERT(renderer != NULL);
    ASSERT(texture_num >= 0);
    ASSERT(texture_num < GFX_MAX_TEXTURES);

    GFX_GL_TEXTURE *const texture = renderer->textures[texture_num];
    if (!texture) {
        LOG_ERROR("Invalid texture handle");
        return false;
    }

    // unbind texture if currently bound
    if (texture_num == renderer->selected_texture_num) {
        M_SelectTextureImpl(renderer, GFX_NO_TEXTURE);
        renderer->selected_texture_num = GFX_NO_TEXTURE;
    }

    GFX_GL_Texture_Free(texture);
    renderer->textures[texture_num] = NULL;
    return true;
}

void GFX_3D_Renderer_RenderPrimStrip(
    GFX_3D_RENDERER *const renderer, const GFX_3D_VERTEX *const vertices,
    const int count)
{
    ASSERT(renderer != NULL);
    ASSERT(vertices != NULL);
    GFX_3D_VertexStream_PushPrimStrip(
        &renderer->vertex_stream, vertices, count);
}

void GFX_3D_Renderer_RenderPrimFan(
    GFX_3D_RENDERER *const renderer, const GFX_3D_VERTEX *const vertices,
    const int count)
{
    ASSERT(renderer != NULL);
    ASSERT(vertices != NULL);
    GFX_3D_VertexStream_PushPrimFan(&renderer->vertex_stream, vertices, count);
}

void GFX_3D_Renderer_RenderPrimList(
    GFX_3D_RENDERER *const renderer, const GFX_3D_VERTEX *const vertices,
    const int count)
{
    ASSERT(renderer != NULL);
    ASSERT(vertices != NULL);
    GFX_3D_VertexStream_PushPrimList(&renderer->vertex_stream, vertices, count);
}

void GFX_3D_Renderer_SelectTexture(
    GFX_3D_RENDERER *const renderer, int texture_num)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    renderer->selected_texture_num = texture_num;
    M_SelectTextureImpl(renderer, texture_num);
}

void GFX_3D_Renderer_SetPrimType(
    GFX_3D_RENDERER *const renderer, GFX_3D_PRIM_TYPE value)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    GFX_3D_VertexStream_SetPrimType(&renderer->vertex_stream, value);
}

void GFX_3D_Renderer_SetTextureFilter(
    GFX_3D_RENDERER *const renderer, GFX_TEXTURE_FILTER filter)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    GFX_GL_Sampler_Parameteri(
        &renderer->sampler, GL_TEXTURE_MAG_FILTER,
        filter == GFX_TF_BILINEAR ? GL_LINEAR : GL_NEAREST);
    GFX_GL_Sampler_Parameteri(
        &renderer->sampler, GL_TEXTURE_MIN_FILTER,
        filter == GFX_TF_BILINEAR ? GL_LINEAR : GL_NEAREST);
    GFX_GL_Program_Uniform1i(
        &renderer->program, renderer->loc_smoothing_enabled,
        filter == GFX_TF_BILINEAR);
}

void GFX_3D_Renderer_SetDepthWritesEnabled(
    GFX_3D_RENDERER *const renderer, const bool is_enabled)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    glDepthMask(is_enabled ? GL_TRUE : GL_FALSE);
}

void GFX_3D_Renderer_SetDepthTestEnabled(
    GFX_3D_RENDERER *const renderer, const bool is_enabled)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    if (is_enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void GFX_3D_Renderer_SetBlendingMode(
    GFX_3D_RENDERER *const renderer, const GFX_BLEND_MODE blend_mode)
{
    ASSERT(renderer != NULL);
    if (renderer->selected_blend_mode == blend_mode) {
        return;
    }
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);

    GFX_GL_Program_Bind(&renderer->program);
    switch (blend_mode) {
    case GFX_BLEND_MODE_OFF:
        glBlendFunc(GL_ONE, GL_ZERO);
        GFX_GL_Program_Uniform1i(
            &renderer->program, renderer->loc_color_key_enabled, false);
        break;
    case GFX_BLEND_MODE_NORMAL:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        GFX_GL_Program_Uniform1i(
            &renderer->program, renderer->loc_color_key_enabled, false);
        break;
    case GFX_BLEND_MODE_MULTIPLY:
        glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        GFX_GL_Program_Uniform1i(
            &renderer->program, renderer->loc_color_key_enabled, false);
        break;
    case GFX_BLEND_MODE_COLOR_KEY:
        glBlendFunc(GL_ONE, GL_ZERO);
        GFX_GL_Program_Uniform1i(
            &renderer->program, renderer->loc_color_key_enabled, true);
        break;
    }
    GFX_GL_CheckError();
    renderer->selected_blend_mode = blend_mode;
}

void GFX_3D_Renderer_SetTexturingEnabled(
    GFX_3D_RENDERER *const renderer, const bool is_enabled)
{
    ASSERT(renderer != NULL);
    GFX_3D_VertexStream_RenderPending(&renderer->vertex_stream);
    GFX_GL_Program_Uniform1i(
        &renderer->program, renderer->loc_texturing_enabled, is_enabled);
}

void GFX_3D_Renderer_SetAnisotropyFilter(
    GFX_3D_RENDERER *const renderer, const float value)
{
    GFX_GL_Sampler_Bind(&renderer->sampler, 0);
    GFX_GL_Sampler_Parameterf(
        &renderer->sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
}
