//purpose: fragment shader with parallax & normal mapping options
//author : ByeongKyu Park (byeonggyu.park)
//date   :  11/11/2023

#version 330 core

uniform sampler2D colorTex;     /*  Base color texture */
uniform sampler2D normalTex;    /*  Normal texture for normal mapping */
uniform sampler2D bumpTex;      /*  Bump texture for bump mapping */

in vec2 uvCoord;

uniform bool lightOn;
uniform int numLights;
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform int specularPower;

/*  These could be in view space or tangent space */
in vec3 lightDir[10];
in vec3 viewDir;
in vec3 normal;

uniform bool normalMappingOn;   /*  whether normal mapping is on */
uniform bool parallaxMappingOn; /*  whether parallax mapping is on */


out vec4 fragColor;

/******************************************************************
 * Applies lighting with optional normal and parallax mapping to 
 * fragment color based on texture and light information.
 ******************************************************************/
void main(void)
{
    if (!lightOn)
    {
        fragColor = vec4(texture(colorTex, uvCoord).rgb, 1.0);
        return;
    }

    vec3 N;  //normal in TBN Space
    vec3 V = normalize(viewDir); //view in TBN Space                                                                                                                          
                                                                                         
    if (!parallaxMappingOn){ //parallax mapping is off
        fragColor = vec4(texture(colorTex, uvCoord).rgb, 1.f);   // uvCoord is used for ALL the texture sampling
    }                                                                
                                                                                         
    if (!normalMappingOn){ //normal mapping is off
        N = normalize(normal); //interpolated normal from the VS shader                                
    }                                                              
    else{
      if (parallaxMappingOn){ //both normal & parallax mapping are on
        float bumpHeight = texture(bumpTex, uvCoord).r * 0.15f - 0.005f;
        vec2 uvCoordAdjusted = uvCoord + bumpHeight * (V.xy/V.z);
        fragColor = vec4(texture(colorTex, uvCoordAdjusted).rgb, 1.f);                                                                                        
        N = normalize(texture(normalTex, uvCoordAdjusted).xyz * 2.f - 1.f);
      }
      else{                 //normal mapping is on, parallax mapping is off                                                                       
        N = normalize(texture(normalTex, uvCoord).xyz * 2.f - 1.f);
      }                        
    }                                                                                                                                                                    

    vec4 intensity = ambient;

    for (int i = 0; i < numLights; ++i){                                                                                      
        vec3 L = normalize(lightDir[i]);                                                     
        vec3 H = normalize(L+V);     
        vec4 diffuseTerm = diffuse * max(dot(L, N), 0.f);
        vec4 specularTerm = specular * pow(max(dot(N, H), 0.f), specularPower); 
        intensity += (diffuseTerm + specularTerm);                                
    }

    fragColor = fragColor * intensity;
}