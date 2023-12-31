#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
#extension GL_ARB_explicit_uniform_location : require

/*  Define inputs of fragment shader: pos, nrm and uv */
in vec2 uvCoord;
in vec3 nrmVF, posVF;

/*  Object texture */
layout (location = 7) uniform sampler2D tex;


/*  Output data of this fragment, which will be written to the framebuffer textures
    and used by the light pass shader for lighting computation.
*/
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragPos;
layout (location = 2) out vec3 fragNrm;
layout (location = 3) out float fragDepth;


void main(void)
{
    /*  Obtain this fragment's color, pos, depth and NORMALIZED normal */
    fragColor = texture(tex, uvCoord);
    fragPos = posVF;
    fragNrm = normalize(nrmVF);
    fragDepth = gl_FragCoord.z;
}
