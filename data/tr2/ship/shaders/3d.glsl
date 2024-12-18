#ifdef VERTEX
// Vertex shader

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTexCoords;
layout(location = 2) in vec4 inColor;

uniform mat4 matProjection;
uniform mat4 matModelView;

#ifdef OGL33C
    out vec4 vertColor;
    out vec3 vertTexCoords;
#else
    varying vec4 vertColor;
    varying vec3 vertTexCoords;
#endif

void main(void) {
    gl_Position = matProjection * matModelView * vec4(inPosition, 1);
    vertColor = inColor / 255.0;
    vertTexCoords = inTexCoords;
}

#else
// Fragment shader

uniform sampler2D tex0;
uniform sampler1D texPalette;
uniform sampler2D texLightMap;
uniform bool texturingEnabled;
uniform bool smoothingEnabled;
uniform bool alphaPointDiscard;
uniform float alphaThreshold;
uniform float brightnessMultiplier;
uniform bool paletteEnabled;

#ifdef OGL33C
    #define OUTCOLOR outColor
    #define TEXTURESIZE textureSize
    #define TEXTURE1D texture
    #define TEXTURE2D texture
    #define TEXTURE3D texture
    #define TEXELFETCH texelFetch

    in vec4 vertColor;
    in vec3 vertTexCoords;
    out vec4 OUTCOLOR;
#else
    #define OUTCOLOR gl_FragColor
    #define TEXTURESIZE textureSize2D
    #define TEXELFETCH texelFetch2D
    #define TEXTURE1D texture1D
    #define TEXTURE2D texture2D
    #define TEXTURE3D texture3D

    varying vec4 vertColor;
    varying vec3 vertTexCoords;
#endif

const float bayer2x2[4] = float[4](
    0.0/4.0, 2.0/4.0,
    3.0/4.0, 1.0/4.0
);

// A 4x4 Bayer dither matrix normalized to [0,1]
const float bayer4x4[16] = float[16](
    0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
   12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
    3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
   15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0
);

void main(void) {
    if (!texturingEnabled) {
        if (paletteEnabled) {
            OUTCOLOR = vertColor * brightnessMultiplier / 2.0;
        } else {
            OUTCOLOR = vertColor * brightnessMultiplier;
        }
        return;
    }

#if defined(GL_EXT_gpu_shader4) || defined(OGL33C)
    if (alphaPointDiscard && smoothingEnabled) {
        // do not use smoothing for chroma key
        ivec2 size = TEXTURESIZE(tex0, 0);
        int tx = int((vertTexCoords.x / vertTexCoords.z) * size.x) % size.x;
        int ty = int((vertTexCoords.y / vertTexCoords.z) * size.y) % size.y;
        vec4 texel = TEXELFETCH(tex0, ivec2(tx, ty), 0);
        if (texel.a == 0.0) {
            discard;
        }
    }
#endif

    if (paletteEnabled) {

        float texIndex = texture(tex0, vertTexCoords.xy / vertTexCoords.z).r;
        float brightness = 1.0 - (vertColor.r * brightnessMultiplier / 2.0);

        // Determine pixel's position in the Bayer matrix
        int bayerSize = 4;
        ivec2 pixelCoord = ivec2(int(gl_FragCoord.x) % bayerSize, int(gl_FragCoord.y) % bayerSize);
        int ditherIndex = pixelCoord.y * bayerSize + pixelCoord.x;
        float ditherVal = bayer4x4[ditherIndex];

        // We assume the `texLightMap` has discrete rows of brightness lines.
        // Let's say it has a continuous gradient, but each row is effectively one brightness step.
        // A small offset in brightness might push the lookup to a darker or lighter tone.
        // Scale the offset so that it spans a fraction of the distance between two brightness lines.
        float brightnessSteps = 16.0;
        float brightnessOffset = (ditherVal - 0.5) * (1.0 / brightnessSteps);

        // Apply the offset so that half of the pixels go slightly darker and half slightly lighter.
        float ditheredBrightness = clamp(brightness + brightnessOffset, 0.0, 1.0);

        // Now fetch from the light map using the same texIndex, but slightly adjusted brightness.
        float paletteVal = texture(texLightMap, vec2(texIndex, ditheredBrightness)).r;

        // Look up the final color in the palette.
        vec3 paletteColor = texture(texPalette, paletteVal).rgb;
        OUTCOLOR = vec4(paletteColor, 1.0);

        if (texIndex == 0) {
            discard;
        }
        if (alphaThreshold >= 0.0 && OUTCOLOR.a <= alphaThreshold) {
            discard;
        }
    } else {
        vec4 texColor = TEXTURE2D(tex0, vertTexCoords.xy / vertTexCoords.z);
        if (alphaThreshold >= 0.0 && texColor.a <= alphaThreshold) {
            discard;
        }
        OUTCOLOR = vec4(OUTCOLOR.rgb * texColor.rgb * brightnessMultiplier, texColor.a);
    }
}
#endif // VERTEX
