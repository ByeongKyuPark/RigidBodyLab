#version 330 core

#define MAX_LIGHTS 10

// Uniforms and layout locations
uniform mat4 mvMat;  // Model-view matrix
uniform mat4 nmvMat; // Normal model-view matrix
uniform mat4 projMat; // Projection matrix
uniform bool normalMappingOn;
uniform int  numLights;
uniform vec3 lightPosVF[MAX_LIGHTS];    /*  light pos already in view frame */
uniform float  objType;      //0   : regular deferred object
                             //1/4 : planar mirror
                             //2/4 : spherical mirror
                             //3/4 : normal mapped plane                             

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nrm;
layout(location = 2) in vec3 tan;
layout(location = 3) in vec3 bitan;
layout(location = 4) in vec2 uv; // Include UV coordinates

out vec3 vPos;
out vec3 vNormal;
out vec2 vUV;
out vec3 vLightDir[MAX_LIGHTS]; //cam space
out vec3 vViewDir;      //cam space
flat out float vFragObjType;

void main() {
    vec4 viewPos = mvMat * vec4(pos, 1.0);
    vPos = viewPos.xyz;
    vUV = uv; // Pass UV coordinates to fragment shader

    gl_Position = projMat * viewPos;

    vViewDir = -viewPos.xyz;                       //'V' (in the cam space)
    vNormal = normalize(mat3(nmvMat) * nrm);    // 'N' (in the cam space)

    if(objType>0.7) //checking plane(=0.75)
    {
        vec3 tVF = normalize(mat3(mvMat) * tan);          //tangent (in the cam space)                              
        vec3 btVF = normalize(mat3(mvMat) * bitan);       //bitangent (in the cam space)   

        mat3 toTBN = transpose(mat3(tVF, btVF, vNormal));

        vViewDir = toTBN * vViewDir;                        //'V' (in TBN space)

        for (int i = 0; i < numLights; ++i) {
            vLightDir[i] = lightPosVF[i] - vPos; //'L' (in the cam space)
            vLightDir[i] = toTBN * vLightDir[i];              //'L' (in TBN space) 
        }
    }
    vFragObjType = objType;    
}
