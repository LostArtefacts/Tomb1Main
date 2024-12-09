#ifdef VERTEX
// Vertex shader

#ifdef OGL33C
    out vec2 vertTexCoords;
    out vec2 vertCoords;
#else
    varying vec2 vertTexCoords;
    varying vec2 vertCoords;
#endif

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoords;

void main(void) {
    gl_Position = vec4(inPosition * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
    vertCoords = inPosition;
    vertTexCoords = inTexCoords;
}

#else
// Fragment shader

#define EFFECT_NONE 0
#define EFFECT_VIGNETTE 1

uniform sampler2D texMain;
uniform sampler1D texPalette;
uniform sampler2D texAlpha;
uniform bool paletteEnabled;
uniform bool alphaEnabled;
uniform int effect;

#ifdef OGL33C
    #define OUTCOLOR outColor
    #define TEXTURE2D texture
    #define TEXTURE1D texture

    in vec2 vertTexCoords;
    in vec2 vertCoords;
    out vec4 outColor;
#else
    #define OUTCOLOR gl_FragColor
    #define TEXTURE2D texture2D
    #define TEXTURE1D texture1D

    varying vec2 vertTexCoords;
    varying vec2 vertCoords;
#endif

void main(void) {
    vec2 uv = vertTexCoords;

    if (alphaEnabled) {
        float alpha = TEXTURE2D(texAlpha, uv).r;
        if (alpha < 0.5) {
            discard;
        }
    }

    if (paletteEnabled) {
        float paletteIndex = TEXTURE2D(texMain, uv).r;
        OUTCOLOR = TEXTURE1D(texPalette, paletteIndex);
    } else {
        OUTCOLOR = TEXTURE2D(texMain, uv);
    }

    if (effect == EFFECT_VIGNETTE) {
        float x_dist = vertCoords.x - 0.5;
        float y_dist = vertCoords.y - 0.5;
        float light = 256 - sqrt(x_dist * x_dist + y_dist * y_dist ) * 300.0;
        light = clamp(light, 0, 255) / 255;
        OUTCOLOR *= vec4(light, light, light, 1);
    }
}
#endif // VERTEX
