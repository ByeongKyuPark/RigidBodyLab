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

uniform bool lightOn;           /*  whether lighting should be applied */
uniform bool normalMappingOn;
uniform bool parallaxMappingOn;
uniform bool forwardRenderOn;

uniform int numLights;
uniform int specularPower;  

uniform vec4 ambient;
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragPos;
layout (location = 2) out vec4 fragNrm;
layout (location = 3) out float fragDepth;

void main(void) {
    fragPos = vPos;
    fragDepth = gl_FragCoord.z;

    fragColor = texture(colorTex, vUV);        

    vec3 N = vNormal;
    vec3 V = normalize(vViewDir); 
                                                                                         
    if (normalMappingOn){
    /*
      This geometry pass (geom_pass.fs) is tailored to handle the lighting computations for the plane. 
      it's to circumvent the need to frequently switch between different shader programs for various objects, streamlining the rendering pipeline. It utilizes a single shader program to address the requirements of both regular objects and the specialized plane.

      Normal and parallax mappings are applied exclusively to the plane object in this scene. 
      This approach optimizes the rendering process by reducing the number of operations 
      required. Instead of performing normal and parallax mapping for every object (O(M*N)), 
      we only apply it to the plane (O(M+N)), where M is the number of objects and N is the 
      number of lightss affecting the plane.

      To facilitate this process, the shader embeds a mask within the alpha channel of the normal vectors. 
      This mask serves as an identifier during the lighting pass, enabling the shader to discern fragments associated with the plane. 
      For these identified fragments, the lighting pass omits any further lighting computations, as these are already processed in the geometry pass.
    */
      if (parallaxMappingOn){ //both normal & parallax mapping are on
        float bumpHeight = texture(bumpTex, vUV).r * 0.2f - 0.005f;
        vec2 uvCoordAdjusted = vUV + bumpHeight * (V.xy);///V.z);
        //fragColor = texture(colorTex, uvCoordAdjusted);                                                                                        
        fragColor = vec4(texture(colorTex, uvCoordAdjusted).rgb, 1.f);                                                                                        
        N = normalize(texture(normalTex, uvCoordAdjusted).xyz * 2.f - 1.f);
      }
      else{                 //normal mapping is on, parallax mapping is off                                                                       
        N = normalize(texture(normalTex, vUV).xyz * 2.f - 1.f);
      }                        
    }           

    if(forwardRenderOn){
      vec4 intensity = ambient;
      
      for (int i = 0; i < numLights; ++i){                                                                                      

        vec3 L = normalize(vLightDir[i]);                                                     
        vec3 H = normalize(L+V);     
        vec4 diffuseTerm = diffuse[i] * max(dot(L, N), 0.f);
        //does blinn phong
        vec4 specularTerm = specular[i] * pow(max(dot(N, H), 0.f), specularPower); 
        intensity += (diffuseTerm + specularTerm);                                
      }
      fragColor *= intensity;// *0.5f;//dimmer

    }

    fragNrm = vec4(N,vFragObjType);
}
