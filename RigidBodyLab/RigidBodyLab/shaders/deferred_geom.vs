#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
#extension GL_ARB_explicit_uniform_location : require

/*  Vertex data */
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec2 uv;


/*  Define the uniform transform matrices:
    - Model-view transform
    - Normal transform
    - Projection transform
    We separate model-view and projection transforms bcoz we want to compute
    lighting in view frame for higher accuracy.
    In world frame, the object positions could be in any range.
    Check graphics.cpp for the correct layout locations to use.
*/
layout (location = 3) uniform mat4 modelViewMat;
layout (location = 4) uniform mat4 normalMat;
layout (location = 5) uniform mat4 projMat;


/*  Define outputs of vertex shader: pos, nrm and uv */
out vec2 uvCoord;
out vec3 nrmVF; // view coord
out vec3 posVF; // view coord


void main(void)
{
    /*	Compute gl_Position, uv, and nrm, pos in appropriate frames  */
    uvCoord = uv;
    vec4 posMV = modelViewMat * vec4(pos, 1.0);
    gl_Position = projMat * posMV;
    nrmVF = mat3(normalMat) * nrm;
    posVF = posMV.xyz;
}