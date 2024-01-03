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

//When objects are rendered using a perspective projection, objects farther from the camera are scaled down more dramatically than objects close to the camera. 
//however, this perspective transformation results in a non-linear distribution of depth values. The depth values are more densely packed near the camera (near plane)
//this means that for two objects that are equally spaced in the 3D world, the difference in their depth values will be smaller if they are far from the camera compared to if they are close.
//since most of the depth values are concentrated near the near plane, most fragments will have depth values very close to 1.0.
//therefore, this function maps the depth values such that they are more evenly distributed between the near and far planes.
float linearizeDepth(float depth) {
    float near = 0.02f; // Near plane distance
    float far = 1.f; // Far plane distance
    return near * far / (far - depth * (far - near));
}

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
                fragColor = vec4(normalize(texture(nrmTex, uvCoord).xyz), 1.0);
                break;                    
            case 4: // DEPTH
                // Linearize the depth value for better visualization
                float linearDepth = linearizeDepth(fragDepth);
                fragColor = vec4(linearDepth, linearDepth, linearDepth, 1.0);
                break;
            case 5: //  NORMAL_MAPPING_OFF (Objects for which normal mapping is not applied)            
                float mask=texture(tanTex, uvCoord).w;
                fragColor = vec4(mask,mask,mask, 1.0);
                break;

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
