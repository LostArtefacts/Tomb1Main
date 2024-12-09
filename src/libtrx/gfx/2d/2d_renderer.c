#include "gfx/2d/2d_renderer.h"

#include "debug.h"
#include "gfx/context.h"
#include "gfx/gl/gl_core_3_3.h"
#include "gfx/gl/utils.h"
#include "log.h"
#include "memory.h"
#include "utils.h"

#include <string.h>

typedef enum {
    M_UNIFORM_TEXTURE_MAIN,
    M_UNIFORM_NUMBER_OF,
} M_UNIFORM;

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat u;
    GLfloat v;
} M_VERTEX;

struct GFX_2D_RENDERER {
    GFX_GL_VERTEX_ARRAY vertex_format;
    GFX_GL_BUFFER surface_buffer;
    GFX_GL_TEXTURE surface_texture;
    GFX_GL_PROGRAM program;

    M_VERTEX *vertices;
    int32_t vertex_count;

    GFX_2D_SURFACE_DESC desc;

    // shader variable locations
    GLint loc[M_UNIFORM_NUMBER_OF];
} M_PRIV;

static const M_VERTEX m_Vertices[] = {
    { .x = 0.0, .y = 0.0, .u = 0.0, .v = 0.0 },
    { .x = 1.0, .y = 0.0, .u = 1.0, .v = 0.0 },
    { .x = 0.0, .y = 1.0, .u = 0.0, .v = 1.0 },
    { .x = 0.0, .y = 1.0, .u = 0.0, .v = 1.0 },
    { .x = 1.0, .y = 0.0, .u = 1.0, .v = 0.0 },
    { .x = 1.0, .y = 1.0, .u = 1.0, .v = 1.0 },
};

static void M_UploadVertices(GFX_2D_RENDERER *const r)
{
    const int32_t mapping[] = { 0, 1, 3, 3, 1, 2 };
    r->vertex_count = 6;
    r->vertices = Memory_Realloc(r->vertices, 6 * sizeof(M_VERTEX));
    M_VERTEX *ptr = r->vertices;

    for (int32_t i = 0; i < 6; i++) {
        ptr->x = m_Vertices[i].x;
        ptr->y = m_Vertices[i].y;
        ptr->u = r->desc.uv[mapping[i]].u;
        ptr->v = r->desc.uv[mapping[i]].v;
        ptr++;
    }

    GFX_GL_Buffer_Bind(&r->surface_buffer);
    GFX_GL_Buffer_Data(
        &r->surface_buffer, sizeof(M_VERTEX) * 6, r->vertices, GL_STATIC_DRAW);
}

GFX_2D_RENDERER *GFX_2D_Renderer_Create(void)
{
    LOG_INFO("");
    GFX_2D_RENDERER *const r = Memory_Alloc(sizeof(GFX_2D_RENDERER));
    const GFX_CONFIG *const config = GFX_Context_GetConfig();

    r->vertices = NULL;
    r->vertex_count = 6;

    GFX_GL_Buffer_Init(&r->surface_buffer, GL_ARRAY_BUFFER);
    GFX_GL_Buffer_Bind(&r->surface_buffer);
    GFX_GL_Buffer_Data(
        &r->surface_buffer, sizeof(m_Vertices), m_Vertices, GL_STATIC_DRAW);

    GFX_GL_VertexArray_Init(&r->vertex_format);
    GFX_GL_VertexArray_Bind(&r->vertex_format);
    GFX_GL_VertexArray_Attribute(
        &r->vertex_format, 0, 2, GL_FLOAT, GL_FALSE, 16, 0);
    GFX_GL_VertexArray_Attribute(
        &r->vertex_format, 1, 2, GL_FLOAT, GL_FALSE, 16, 8);
    GFX_GL_CheckError();

    GFX_GL_Texture_Init(&r->surface_texture, GL_TEXTURE_2D);

    GFX_GL_Program_Init(&r->program);
    GFX_GL_Program_AttachShader(
        &r->program, GL_VERTEX_SHADER, "shaders/2d.glsl", config->backend);
    GFX_GL_Program_AttachShader(
        &r->program, GL_FRAGMENT_SHADER, "shaders/2d.glsl", config->backend);
    GFX_GL_Program_Link(&r->program);
    GFX_GL_Program_FragmentData(&r->program, "fragColor");

    struct {
        M_UNIFORM loc;
        const char *name;
    } uniforms[] = {
        { M_UNIFORM_TEXTURE_MAIN, "texMain" },
        { -1, NULL },
    };
    for (int32_t i = 0; uniforms[i].name != NULL; i++) {
        r->loc[uniforms[i].loc] =
            GFX_GL_Program_UniformLocation(&r->program, uniforms[i].name);
        GFX_GL_CheckError();
    }

    GFX_GL_Program_Bind(&r->program);
    GFX_GL_Program_Uniform1i(&r->program, r->loc[M_UNIFORM_TEXTURE_MAIN], 0);
    GFX_GL_CheckError();

    return r;
}

void GFX_2D_Renderer_Destroy(GFX_2D_RENDERER *const r)
{
    ASSERT(r != NULL);

    GFX_GL_VertexArray_Close(&r->vertex_format);
    GFX_GL_Buffer_Close(&r->surface_buffer);
    GFX_GL_Texture_Close(&r->surface_texture);
    GFX_GL_Program_Close(&r->program);
    Memory_FreePointer(&r->vertices);
    Memory_Free(r);
}

void GFX_2D_Renderer_UploadSurface(
    GFX_2D_RENDERER *const r, GFX_2D_SURFACE *const surface)
{
    GFX_2D_Renderer_Upload(r, &surface->desc, surface->buffer);
}

void GFX_2D_Renderer_Upload(
    GFX_2D_RENDERER *const r, GFX_2D_SURFACE_DESC *const desc,
    const uint8_t *const data)
{
    ASSERT(r != NULL);

    bool reupload_vert = false;
    if (memcmp(r->desc.uv, desc->uv, sizeof(desc->uv)) != 0) {
        reupload_vert = true;
    }

    glActiveTexture(GL_TEXTURE0);
    GFX_GL_Texture_Bind(&r->surface_texture);

    // update buffer if the size is unchanged, otherwise create a new one
    if (r->desc.width != desc->width || r->desc.height != desc->height
        || r->desc.tex_format != desc->tex_format
        || r->desc.tex_type != desc->tex_type) {
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, desc->width, desc->height, 0,
            desc->tex_format, desc->tex_type, data);
        GFX_GL_CheckError();
    } else {
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, desc->width, desc->height, desc->tex_format,
            desc->tex_type, data);
        GFX_GL_CheckError();
    }

    r->desc = *desc;
    if (reupload_vert) {
        M_UploadVertices(r);
    }
}

void GFX_2D_Renderer_Render(GFX_2D_RENDERER *const r)
{
    ASSERT(r != NULL);

    GFX_GL_Program_Bind(&r->program);
    GFX_GL_Buffer_Bind(&r->surface_buffer);
    GFX_GL_VertexArray_Bind(&r->vertex_format);

    glActiveTexture(GL_TEXTURE0);
    GFX_GL_Texture_Bind(&r->surface_texture);

    GLboolean blend = glIsEnabled(GL_BLEND);
    if (blend) {
        glDisable(GL_BLEND);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
    if (depth_test) {
        glDisable(GL_DEPTH_TEST);
    }

    glDrawArrays(GL_TRIANGLES, 0, r->vertex_count);
    GFX_GL_CheckError();

    if (blend) {
        glEnable(GL_BLEND);
    }

    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    }
    glActiveTexture(GL_TEXTURE0);
}
