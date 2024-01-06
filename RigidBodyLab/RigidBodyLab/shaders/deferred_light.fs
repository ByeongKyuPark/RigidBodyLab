#version 330 core

uniform int lightPassDebug;

// G-buffer textures
uniform sampler2D posTex;
uniform sampler2D nrmTex;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform sampler2D shadowMapDepthTex;

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
uniform mat4 lightSpaceMat;

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
    depth = depth * 2.0 - 1.0; // Back to NDC 
    return near * far / (far - depth * (far - near));
    return (2.0 * near * far) / (far + near - depth * (far - near));    
}

float linearizeDepth2(float depth) {
    float near = 0.005f; // Near plane distance
    float far = 10.f; // Far plane distance
    depth = depth * 2.0 - 1.0; // Back to NDC 
    return near * far / (far - depth * (far - near));
    return (2.0 * near * far) / (far + near - depth * (far - near));    
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMapDepthTex, projCoords.xy).r; 
    closestDepth = linearizeDepth2(closestDepth);

    // get depth of current fragment from light's perspective
    float currentDepth = linearizeDepth(projCoords.z);
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    //return shadow;    
    //return currentDepth;
    return closestDepth;
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
            case 5: //  NORMAL_MAPPING (Objects for which normal mapping is applied)            
                /*
                if (fragDepth >= 0.999f) { //background                
                    fragColor = vec4(0.f,0.f,0.f, 1.f);
                }
                else{
                    float mask=texture(nrmTex, uvCoord).w;
                    //reverse black <-> white
                    //mask = mask>0.5 ? 0.f: 1.f;                    
                    fragColor = vec4(mask,mask,0.f, 1.f);//yellow
                }                
                */
                /*
                float depth = texture(shadowMapDepthTex, uvCoord).r;
                depth = linearizeDepth(depth);                
                fragColor = vec4(depth, depth, depth, 1.0);
                */
                float depth = texture(shadowMapDepthTex, uvCoord).r;
                depth = linearizeDepth(depth);

                fragColor = vec4(vec3(depth),1.0);
                break;

        }
    } else {

        fragColor = texture(colorTex, uvCoord);

        if (fragDepth >= 0.999f) { //background
            return;
        }

        vec3 fragPos = texture(posTex, uvCoord).xyz;
        vec4 nrmPack = texture(nrmTex, uvCoord);
        vec3 normal = normalize(nrmPack.xyz);
        vec3 viewDir = normalize(-fragPos);
        float objectType = nrmPack.w; //0   : regular deferred object
                                     //1/4 : planar mirror
                                     //2/4 : spherical mirror
                                     //3/4 : normal mapped plane                             

        vec4 fragPosLightSpace = lightSpaceMat * vec4(fragPos, 1.0);        
        float shadow = ShadowCalculation(fragPosLightSpace);      
        //fragColor = fragPosLightSpace;
        fragColor =vec4(vec3(shadow),1.f);

        return;
        
        //vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color; 
        vec4 intensity = ambient ;

        if (objectType > 0.7f) {// plane
            return; //light already computed
        }
        else if(objectType>0.2f){//planar & spherical mirror
            intensity += vec4(0.27,0.27,0.27,1.f); //brighter (optional, for demonstration purposes)            
        }

        for (int i = 0; i < numLights; ++i) {
            vec3 lightDir = normalize(normalize(lightPosVF[i] - fragPos));
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
