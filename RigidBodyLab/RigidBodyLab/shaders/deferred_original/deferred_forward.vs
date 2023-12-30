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
layout (location = 3) uniform mat4 mvMat;
layout (location = 4) uniform mat4 normalMat;
layout (location = 5) uniform mat4 projMat;

/*  numLights and lightPosVF */
layout (location = 8) uniform int  numLights;
layout (location = 9) uniform vec3 lightPosVF[10];  /*	Some drivers only support these many elements */
layout (location = 24) uniform int  blinnPhongLighting;


out vec2 uvCoord;

/*  These output vectors are in view frame */
out vec3 lightVF[10];           /*  Light vector from vertex to light source */
out vec3 nrmVF;                 /*  Vertex normal */
out vec3 viewVF;                /*  View normal from vertex to camera */

void main(void) 
{
    /*  Vertex position in view frame */
    vec4 posVF = mvMat * vec4(pos, 1.0);

    /*	For lighting: computing normal, view and light vectors in view frame */

    nrmVF = normalize(mat3(normalMat) * nrm); // normal vector to the view space
    viewVF = -normalize(vec3(posVF)); // direction from vertex to the camera in view frame

    for(int i = 0; i < numLights; ++i)
    {
        lightVF[i] = normalize(lightPosVF[i] - vec3(posVF)); // direction from vertex to each light
    }

	/*	For object transformation */
    gl_Position = projMat * posVF;

    /*  For object texturing */
    uvCoord = uv;
}