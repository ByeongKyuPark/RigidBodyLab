#version 330 core

/*  This is required for GLSL below 4.3 to explicitly define uniform variable locations */
#extension GL_ARB_explicit_uniform_location : require

/*  Object texture */
layout(location = 7) uniform sampler2D tex;

in vec2 uvCoord;

/*  numLights and light properties */
layout (location = 8) uniform int numLights;
layout (location = 20) uniform vec4 ambient;
layout (location = 21) uniform vec4 diffuse;
layout (location = 22) uniform vec4 specular;
layout (location = 23) uniform int specularPower;
layout (location = 24) uniform int blinnPhongLighting;  // 1 for active, 0 for inactive

/*  Input vectors in view frame */
in vec3 lightVF[10];
in vec3 nrmVF;
in vec3 viewVF;

out vec4 outColor;

void main(void)
{
    outColor = vec4(texture(tex, uvCoord).rgb, 1.0);

    /*  Compute lighting using Phong equation */
    vec4 intensity = ambient; 
    
    vec3 Nrm = normalize(nrmVF);
    vec3 View = normalize(viewVF);

    for(int i = 0; i < numLights; ++i)
    {
        //assuming spotlight coefficient 1 & no attenuation.
        vec3 L = normalize(lightVF[i]);

        //diffuse
        intensity += diffuse * max(dot(Nrm, L), 0.0);
        //intensity
        if(blinnPhongLighting == 1){//active
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