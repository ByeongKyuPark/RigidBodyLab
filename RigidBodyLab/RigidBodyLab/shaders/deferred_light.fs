#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
//#extension GL_ARB_explicit_uniform_location : require

/*  This is for the shader to know whether to compute lighting, or just display
    color/position/normal/depth texture.
*/
uniform int lightPassDebug;   

/*  The color/pos/nrm/depth textures to be sampled for lighting computation */
uniform sampler2D colorTex;
uniform sampler2D posTex;
uniform sampler2D nrmTex;
uniform sampler2D depthTex;

/*  numLights and lightPosVF */

#define MAX_LIGHTS 10
// Uniforms for number of lights and their positions in the view frame
uniform int numLights;
uniform vec3 lightPosVF[MAX_LIGHTS];

/*  Light intensities and properties */
uniform vec4 ambient;
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform int specularPower;
uniform int blinnPhongLighting;  // 1 for active, 0 for inactive

in vec2 uvCoord;

out vec4 outColor;

void main(void)
{    
    //vec3 position = texture(posTex, uvCoord).xyz;
    //outColor = vec4(position, 1.0);
    //vec3 nrm = texture(nrmTex, uvCoord).xyz;
    //outColor = vec4(nrm, 1.0);
    //vec3 col = texture(colorTex, uvCoord).xyz;
    //outColor = vec4(pos, 1.0);
    //return;

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
            intensity += diffuse[i] * max(dot(Nrm, L), 0.0); //temp
            // specular
            if(blinnPhongLighting==1){//active
                vec3 H = normalize(L + View);  
                intensity += specular[i] * pow(max(dot(Nrm, H), 0.0), specularPower);//temp
            }
            else{
                vec3 Ref = reflect(-L, Nrm);
                intensity += specular[i] * pow(max(dot(Ref, View), 0.0), specularPower);//temp
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
        outColor = vec4(fragDepth, fragDepth, fragDepth, 1.0);
    }
}