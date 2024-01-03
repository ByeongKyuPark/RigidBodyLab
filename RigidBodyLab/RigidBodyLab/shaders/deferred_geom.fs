#version 330 core

#extension GL_ARB_explicit_uniform_location : require

in vec2 fragUV;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragTan;
in float fragObjType; 

uniform sampler2D colorTex;  // Base color texture

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragPosOut;
layout (location = 2) out vec3 fragNrmOut;
layout (location = 3) out vec4 fragTanOut;
layout (location = 4) out float fragDepth;

void main(void) {
    fragColor = texture(colorTex, fragUV);
    fragPosOut = fragPos;
    fragNrmOut = fragNormal;
    fragTanOut = vec4(fragTan,fragObjType);//pack the object type ID into the alpha channel of tangent
    fragDepth = gl_FragCoord.z;
}
