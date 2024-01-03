#version 330 core

uniform int lightPassDebug;

// G-buffer textures
uniform sampler2D posTex;
uniform sampler2D nrmTex;
uniform sampler2D colorTex;
uniform sampler2D tanTex;
uniform sampler2D depthTex;

// Light properties
#define MAX_LIGHTS 10
uniform vec3 lightPosVF[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec4 diffuse[MAX_LIGHTS];
uniform vec4 specular[MAX_LIGHTS];
uniform bool parallaxMappingOn;
uniform int numLights;
uniform int specularPower;  
uniform int blinnPhongLighting;  // 1 for active, 0 for inactive
uniform int normalMappingObjType; // Object type for normal mapping

in vec2 uvCoord;
out vec4 fragColor;

void main(void) {
    float fragDepth = texture(depthTex, uvCoord).r;
    if (lightPassDebug != 0) {
        // Handle non-lighting debug modes
        switch(lightPassDebug) {
            case 1: // COLOR
                fragColor = texture(colorTex, uvCoord);
                break;
            case 2: // POSITION
                fragColor = texture(posTex, uvCoord);
                break;
            case 3: // NORMAL
                {
                fragColor = vec4(normalize(texture(nrmTex, uvCoord).xyz), 1.0);
                break;                    
                }
            case 4: // DEPTH
                fragColor = vec4(fragDepth, fragDepth, fragDepth, 1.0);
            //case 4: // TANGENT
                //fragColor = vec4(normalize(texture(tanTex, uvCoord).xyz), 1.0);
                break;
            // Add other debug modes if necessary
        }
    } else {

        fragColor = texture(colorTex, uvCoord);

        if (fragDepth >= 0.999f) { //background
            return;
        }

        vec3 fragPos = texture(posTex, uvCoord).xyz; //cam space
        vec3 normal = normalize(texture(nrmTex, uvCoord).xyz); //cam space
        vec3 viewDir = normalize(normalize(-fragPos)); //cam space
        vec4 tan = texture(tanTex, uvCoord);
        float objectType = tan.w;

        mat3 TBN = mat3(1.0); // Identity matrix as default

        //convert into TBN space
        if (objectType < 0.5f) {// check if the object is a normal mapped plane
            vec3 tangent = normalize(tan.xyz); // Tangent in camera space
            vec3 bitangent = cross(normal, tangent); // Bitangent in camera space
            TBN = mat3(tangent, bitangent, normal);
            normal = normalize(TBN * normal); // Convert normal to tangent space
            viewDir = normalize(TBN * viewDir); // Convert view direction to tangent space
        }

        vec4 intensity = ambient;
        
        for (int i = 0; i < numLights; ++i) {
            vec3 lightDir = TBN*normalize(normalize(lightPosVF[i] - fragPos));
            //diffuse
            intensity += diffuse[i]* max(dot(normal, lightDir), 0.0);

            //specular
            if(blinnPhongLighting==1){//blinn phong
                vec3 H = normalize(lightDir+viewDir);
                intensity += specular[i]* pow(max(dot(H, normal), 0.0), specularPower);
            }
            else{//normal phong
                vec3 reflectDir = reflect(-lightDir, normal);
                intensity += specular[i]* pow(max(dot(viewDir, reflectDir), 0.0), specularPower);
            }
            fragColor *= intensity;
        }
    }
}
