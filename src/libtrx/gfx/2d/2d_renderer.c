#include "gfx/2d/2d_renderer.h"

#include "debug.h"
#include "gfx/context.h"
#include "gfx/gl/gl_core_3_3.h"
#include "gfx/gl/utils.h"
#include "log.h"
#include "memory.h"

struct GFX_2D_RENDERER {
    uint32_t width;
    uint32_t height;
    GFX_GL_VERTEX_ARRAY surface_format;
    GFX_GL_BUFFER surface_buffer;
    GFX_GL_TEXTURE surface_texture;
    GFX_GL_PROGRAM program;
};

GFX_2D_RENDERER *GFX_2D_Renderer_Create(void)
{
    LOG_INFO("");
    GFX_2D_RENDERER *const r = Memory_Alloc(sizeof(GFX_2D_RENDERER));
    const GFX_CONFIG *const config = GFX_Context_GetConfig();

    GFX_GL_Buffer_Init(&r->surface_buffer, GL_ARRAY_BUFFER);
    GFX_GL_Buffer_Bind(&r->surface_buffer);
    GLfloat verts[] = { 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
                        0.0, 1.0, 1.0, 0.0, 1.0, 1.0 };
    GFX_GL_Buffer_Data(
        &r->surface_buffer, sizeof(verts), verts, GL_STATIC_DRAW);

    GFX_GL_VertexArray_Init(&r->surface_format);
    GFX_GL_VertexArray_Bind(&r->surface_format);
    GFX_GL_VertexArray_Attribute(
        &r->surface_format, 0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GFX_GL_Texture_Init(&r->surface_texture, GL_TEXTURE_2D);

    GFX_GL_Program_Init(&r->program);
    GFX_GL_Program_AttachShader(
        &r->program, GL_VERTEX_SHADER, "shaders/2d.glsl", config->backend);
    GFX_GL_Program_AttachShader(
        &r->program, GL_FRAGMENT_SHADER, "shaders/2d.glsl", config->backend);
    GFX_GL_Program_Link(&r->program);
    GFX_GL_Program_FragmentData(&r->program, "fragColor");

    GFX_GL_CheckError();
    return r;
}

void GFX_2D_Renderer_Destroy(GFX_2D_RENDERER *const r)
{
    LOG_INFO("");
    ASSERT(r != NULL);

    GFX_GL_VertexArray_Close(&r->surface_format);
    GFX_GL_Buffer_Close(&r->surface_buffer);
    GFX_GL_Texture_Close(&r->surface_texture);
    GFX_GL_Program_Close(&r->program);
    Memory_Free(r);
}

void GFX_2D_Renderer_Upload(
    GFX_2D_RENDERER *const r, GFX_2D_SURFACE_DESC *const desc,
    const uint8_t *const data)
{
    ASSERT(r != NULL);
    const uint32_t width = desc->width;
    const uint32_t height = desc->height;

    GFX_GL_Texture_Bind(&r->surface_texture);

    // TODO: implement texture packs

    // update buffer if the size is unchanged, otherwise create a new one
    if (width != r->width || height != r->height) {
        r->width = width;
        r->height = height;
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, r->width, r->height, 0, desc->tex_format,
            desc->tex_type, data);
        GFX_GL_CheckError();
    } else {
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, r->width, r->height, desc->tex_format,
            desc->tex_type, data);
        GFX_GL_CheckError();
    }
}

void GFX_2D_Renderer_Render(GFX_2D_RENDERER *const r)
{
    ASSERT(r != NULL);
    GFX_GL_Program_Bind(&r->program);
    GFX_GL_Buffer_Bind(&r->surface_buffer);
    GFX_GL_VertexArray_Bind(&r->surface_format);
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

    glDrawArrays(GL_TRIANGLES, 0, 6);
    GFX_GL_CheckError();

    if (blend) {
        glEnable(GL_BLEND);
    }

    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    }
}
