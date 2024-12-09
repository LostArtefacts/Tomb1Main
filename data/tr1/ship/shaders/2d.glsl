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

uniform sampler2D texMain;

#ifdef OGL33C
    #define OUTCOLOR outColor
    #define TEXTURE texture

    in vec2 vertTexCoords;
    in vec2 vertCoords;
    out vec4 outColor;
#else
    #define OUTCOLOR gl_FragColor
    #define TEXTURE texture2D

    varying vec2 vertTexCoords;
    varying vec2 vertCoords;
#endif

void main(void) {
    OUTCOLOR = TEXTURE(texMain, vertTexCoords);
}
#endif // VERTEX
