#version 330 core

#extension GL_ARB_explicit_uniform_location : require

in vec2 fragUV;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragTan;

uniform sampler2D colorTex;  // Base color texture

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragPosOut;
layout (location = 2) out vec3 fragNrmOut;
layout (location = 3) out vec3 fragTanOut;
layout (location = 4) out float fragDepth;

void main(void) {
    fragColor = texture(colorTex, fragUV);
    fragPosOut = fragPos;
    fragNrmOut = fragNormal;
    fragTanOut = fragTan;
    fragDepth = gl_FragCoord.z;
}
