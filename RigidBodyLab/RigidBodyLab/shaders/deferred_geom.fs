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
uniform sampler2D colorTex;
uniform sampler2D normalTex;    /*  Normal texture for normal mapping */
uniform sampler2D bumpTex;      /*  Bump texture for bump mapping */

uniform bool lightOn;           /*  whether lighting should be applied */
uniform bool normalMappingOn;   /*  whether normal mapping is on */
uniform bool parallaxMappingOn; /*  whether parallax mapping is on */

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
    fragColor = texture(colorTex, uvCoord);
    fragPos = position;
    fragNrm = normal;
    fragDepth = gl_FragCoord.z;
    //-------------------
    if (!lightOn)
    {
        fragColor = vec4(texture(colorTex, uvCoord).rgb, 1.0);
        fragNrm = normal;
        return;
    }

    vec3 V = normalize(viewDir); //view in TBN Space                                                                                                                          
                                                                                         
    if (!parallaxMappingOn){ //parallax mapping is off
        fragColor = vec4(texture(colorTex, uvCoord).rgb, 1.f);   // uvCoord is used for ALL the texture sampling
    }                                                                
                                                                                         
    if (!normalMappingOn){ //normal mapping is off
        fragNrm = normal; //interpolated normal from the VS shader       
    }                                                              
    else{
      if (parallaxMappingOn){ //both normal & parallax mapping are on
        float bumpHeight = texture(bumpTex, uvCoord).r * 0.21f - 0.005f;
        vec2 uvCoordAdjusted = uvCoord + bumpHeight * (V.xy);///V.z);
        fragColor = vec4(texture(colorTex, uvCoordAdjusted).rgb, 1.f);                                                                                        
        fragNrm = normalize(texture(normalTex, uvCoordAdjusted).xyz * 2.f - 1.f);
      }
      else{                 //normal mapping is on, parallax mapping is off                                                                       
        fragNrm = normalize(texture(normalTex, uvCoord).xyz * 2.f - 1.f);
      }                        
    }   
}
