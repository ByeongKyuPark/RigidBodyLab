#version 330 core

//#extension GL_ARB_explicit_uniform_location : require

#define MAX_LIGHTS 10

in vec3 vPos;   
in vec3 vNormal;
in vec2 vUV;     
in vec3 vLightDir[MAX_LIGHTS]; //cam space
in vec3 vViewDir;      //cam space
in float vFragObjType;   

out vec4 fColor;
out vec3 fPos; 
out vec4 fNormal; 
out float fDepth;

uniform sampler2D colorTex;
uniform sampler2D normalTex;  
uniform sampler2D bumpTex;

uniform bool normalMappingOn;
uniform bool parallaxMappingOn;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragPos;
layout (location = 2) out vec4 fragNrm;
layout (location = 3) out float fragDepth;

void main(void) {
    fragColor = texture(colorTex, vUV);
    fragPos = vPos;
    //fragNrmOut = vec4(vNormal,vFragObjType);//pack the object type ID into the alpha channel
    fragDepth = gl_FragCoord.z;

    vec3 V = normalize(vViewDir); 

    if (!parallaxMappingOn){ //parallax mapping is off
        fragColor = vec4(texture(colorTex, vUV).rgb, 1.f);   // uvCoord is used for ALL the texture sampling
    }                                                                
                                                                                         
    if (!normalMappingOn){ //normal mapping is off
        fragNrm =  vec4(vNormal,vFragObjType); //interpolated normal from the VS shader                                
    }                                                              
    else{
      if (parallaxMappingOn){ //both normal & parallax mapping are on
        float bumpHeight = texture(bumpTex, vUV).r * 0.15f - 0.005f;
        vec2 uvCoordAdjusted = vUV + bumpHeight * (V.xy/V.z);
        fragColor = vec4(texture(colorTex, uvCoordAdjusted).rgb, 1.f);                                                                                        
        fragNrm = vec4(normalize(texture(normalTex, uvCoordAdjusted).xyz * 2.f - 1.f), vFragObjType);
      }
      else{                 //normal mapping is on, parallax mapping is off                                                                       
        fragNrm = vec4(normalize(texture(normalTex, vUV).xyz * 2.f - 1.f), vFragObjType);
      }                        
    }       
}
