#include "gfx/gl/sampler.h"

#include "debug.h"
#include "gfx/gl/utils.h"

void GFX_GL_Sampler_Init(GFX_GL_SAMPLER *sampler)
{
    ASSERT(sampler != NULL);
    glGenSamplers(1, &sampler->id);
    GFX_GL_CheckError();
    sampler->initialized = true;
}

void GFX_GL_Sampler_Close(GFX_GL_SAMPLER *sampler)
{
    ASSERT(sampler != NULL);
    if (sampler->initialized) {
        glDeleteSamplers(1, &sampler->id);
        GFX_GL_CheckError();
    }
    sampler->initialized = false;
}

void GFX_GL_Sampler_Bind(GFX_GL_SAMPLER *sampler, GLuint unit)
{
    ASSERT(sampler != NULL);
    ASSERT(sampler->initialized);
    glBindSampler(unit, sampler->id);
    GFX_GL_CheckError();
}

void GFX_GL_Sampler_Parameteri(
    GFX_GL_SAMPLER *sampler, GLenum pname, GLint param)
{
    ASSERT(sampler != NULL);
    ASSERT(sampler->initialized);
    glSamplerParameteri(sampler->id, pname, param);
    GFX_GL_CheckError();
}

void GFX_GL_Sampler_Parameterf(
    GFX_GL_SAMPLER *sampler, GLenum pname, GLfloat param)
{
    ASSERT(sampler != NULL);
    ASSERT(sampler->initialized);
    glSamplerParameterf(sampler->id, pname, param);
    GFX_GL_CheckError();
}
