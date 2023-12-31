#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
#extension GL_ARB_explicit_uniform_location : require

/*  This is for the shader to know whether to compute lighting, or just display
    color/position/normal/depth texture.
*/
layout (location = 1) uniform int lightPassDebug;   

/*  The color/pos/nrm/depth textures to be sampled for lighting computation */
layout (location = 2) uniform sampler2D colorTex;
layout (location = 3) uniform sampler2D posTex;
layout (location = 4) uniform sampler2D nrmTex;
layout (location = 5) uniform sampler2D depthTex;

/*  numLights and lightPosVF */

#define MAX_LIGHTS 10
// Uniforms for number of lights and their positions in the view frame
layout (location = 8) uniform int numLights;
layout (location = 9) uniform vec3 lightPosVF[MAX_LIGHTS];

/*  Light intensities and properties */
layout (location = 20) uniform vec4 ambient;
layout (location = 21) uniform vec4 diffuse;
layout (location = 22) uniform vec4 specular;
layout (location = 23) uniform int specularPower;
layout (location = 24) uniform int blinnPhongLighting;  // 1 for active, 0 for inactive

in vec2 uvCoord;

out vec4 outColor;

void main(void)
{
    /*  Check lightPassDebug to see if it's 0 (MAIN), 1 (COLOR), 
        2 (POSITION), 3 (NORMAL) or 4 (DEPTH) and proceed accordingly.
        If it's MAIN then compute lighting using Phong equation.
        Otherwise just sample the appropriate texture.
    */
    float fragDepth = texture(depthTex, uvCoord).r;
    
    if (lightPassDebug == 0)//MAIN
    {
        outColor = texture(colorTex, uvCoord);

        if (fragDepth >= 0.999f) { //background
            return;
        }

        vec3 fragPos = texture(posTex, uvCoord).xyz;
        vec3 Nrm = normalize(texture(nrmTex, uvCoord).xyz);
        vec3 View = normalize(-fragPos);
        vec4 intensity = ambient;

        for (int i = 0; i < numLights; ++i)
        {
            // phong lighting
            vec3 L = normalize(lightPosVF[i] - fragPos);

            // diffuse            
            intensity += diffuse * max(dot(Nrm, L), 0.0);
            // specular
            if(blinnPhongLighting==1){//active
                vec3 H = normalize(L + View);  
                intensity += specular * pow(max(dot(Nrm, H), 0.0), specularPower);
            }
            else{
                vec3 Ref = reflect(-L, Nrm);
                intensity += specular * pow(max(dot(Ref, View), 0.0), specularPower);
            }
        }
        outColor *= intensity;
    }
    else if (lightPassDebug == 1) //COLOR
    {   
        outColor = texture(colorTex, uvCoord);
    }
    else if (lightPassDebug == 2) //POSITION
    {
        outColor = vec4(texture(posTex, uvCoord).rgb,1.f);
    }
    else if (lightPassDebug == 3) //NORMAL
    {
        vec3 normalizedNormal = texture(nrmTex, uvCoord).rgb;
        outColor = vec4(normalizedNormal, 1.0);
    }
    else if (lightPassDebug == 4) //DEPTH
    {
        float normalizedDepth = fragDepth;
        outColor = vec4(normalizedDepth, normalizedDepth, normalizedDepth, 1.0);
    }
}