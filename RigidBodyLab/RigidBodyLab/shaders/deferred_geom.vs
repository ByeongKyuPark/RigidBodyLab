#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
#extension GL_ARB_explicit_uniform_location : require

/*  Vertex data */
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec3 tan;
layout (location = 3) in vec3 bitan;
layout (location = 4) in vec2 uv;


/*  Define the uniform transform matrices:
    - Model-view transform
    - Normal transform
    - Projection transform
    We separate model-view and projection transforms bcoz we want to compute
    lighting in view frame for higher accuracy.
    In world frame, the object positions could be in any range.
    Check graphics.cpp for the correct layout locations to use.
*/
uniform mat4 mvMat;
uniform mat4 nmvMat;
uniform mat4 projMat;

uniform bool lightOn;           /*  whether lighting should be applied */
uniform int  numLights;
uniform vec3 lightPosVF[10];    /*  light pos already in view frame */

uniform bool normalMappingOn;   /*  whether normal mapping should be applied */

/*  Define outputs of vertex shader: pos, nrm and uv */
out vec2 uvCoord;
//out vec3 nrmVF; // view coord
//out vec3 posVF; // view coord

out vec3 lightDir[10]; //cam space
out vec3 viewDir;      //cam space
out vec3 normal;       //cam space (nrmVF)
out vec3 position;        //cam space coord

void main(void)
{
    vec4 posVF = mvMat * vec4(pos, 1.0);
    position=posVF.xyz;
        /*    For object transformation */
    gl_Position = projMat * posVF;

    /*  For object texturing */
    uvCoord = uv;

    if (lightOn)
    {
        viewDir = -posVF.xyz;                       //'V' (in the cam space)
        for (int i = 0; i < numLights; ++i){
          lightDir[i] = lightPosVF[i] - posVF.xyz; //'L' (in the cam space)
        }
        normal = normalize(mat3(nmvMat) * nrm);    // 'N' (in the cam space)

        if (normalMappingOn)    
        {
          vec3 tVF = normalize(mat3(mvMat) * tan);          //tangent (in the cam space)                              
          vec3 btVF = normalize(mat3(mvMat) * bitan);       //bitangent (in the cam space)   
                                                                                      
          mat3 toTBN = transpose(mat3(tVF, btVF, normal));                             
                                                                                      
          viewDir = toTBN * viewDir;                        //'V' (in TBN space)
                                                                                      
          for (int i = 0; i < numLights; ++i){
            lightDir[i] = toTBN * lightDir[i];              //'L' (in TBN space) 
          }                                             
        }
    }
}