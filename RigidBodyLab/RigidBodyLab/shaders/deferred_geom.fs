#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
#extension GL_ARB_explicit_uniform_location : require

const int NumMaxLights = 10;

/*  Define inputs of fragment shader: pos, nrm and uv */
in vec2 uvCoord;

in vec3 lightDir[NumMaxLights]; //cam space
in vec3 viewDir;      //cam space
in vec3 normal;       //cam space (nrmVF)
in vec3 position;        //cam space coord

/*  Object texture */
uniform sampler2D tex;//7


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
    fragColor = vec4(0.f, 0.f, 1.f,1.f); 
    //fragColor = texture(tex, uvCoord);
    //fragPos = position;
    fragPos = vec3(1.f, 0.f, 0.f); 
    //fragNrm = normal;
    fragNrm = vec3(0.f, 1.f, 0.f); 
    fragDepth = gl_FragCoord.z;
}
