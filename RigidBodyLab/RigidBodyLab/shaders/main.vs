//purpose: vertex shader that transforms attributes in model space to camera & tangent space
//author : ByeongKyu Park (byeonggyu.park)
//date   :  11/11/2023

#version 330 core

/*  These vertex attributes are in model space */
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec3 tan;
layout (location = 3) in vec3 bitan;
layout (location = 4) in vec2 uv;

uniform mat4 mvMat;     /*  model-view matrix for positions */
uniform mat4 nmvMat;    /*  model-view matrix for normals */
uniform mat4 projMat;   /*  projection matrix */

uniform bool lightOn;           /*  whether lighting should be applied */
uniform int  numLights;
uniform vec3 lightPosVF[10];    /*  light pos already in view frame */

uniform bool normalMappingOn;   /*  whether normal mapping should be applied */


out vec2 uvCoord;

/*  Output vectors:
    - If normalMapping is on then these vectors are in tangent space.
    - Otherwise they are in view space
*/
out vec3 lightDir[10]; //cam space
out vec3 viewDir;      //cam space
out vec3 normal;       //cam space

/******************************************************************
 * Transform vertex data to camera space, construct TBN matrix, and 
 * transform light and view vectors to tangent space.
 ******************************************************************/
void main(void) 
{
    vec4 posVF = mvMat * vec4(pos, 1.0);

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