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
uniform vec4 lightColor[MAX_LIGHTS];

// Camera view position
uniform vec3 viewPos;

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
                fragColor = vec4(normalize(texture(nrmTex, uvCoord).xyz), 1.0);
                break;
            case 4: // DEPTH
                fragColor = vec4(fragDepth, fragDepth, fragDepth, 1.0);
            //case 4: // TANGENT
                //fragColor = vec4(normalize(texture(tanTex, uvCoord).xyz), 1.0);
                break;
            // Add other debug modes if necessary
        }
    } else {
        if (fragDepth >= 0.999f) { //background
            fragColor = texture(colorTex, uvCoord);
            return;
        }

        vec3 fragPos = texture(posTex, uvCoord).xyz;
        vec3 normal = normalize(texture(nrmTex, uvCoord).xyz);
        vec3 tangent = normalize(texture(tanTex, uvCoord).xyz);
        vec3 bitangent = cross(normal, tangent);
        mat3 TBN = mat3(tangent, bitangent, normal);

        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 viewDirTBN = TBN * viewDir;

        vec3 lighting = vec3(0.0);
        for (int i = 0; i < MAX_LIGHTS; ++i) {
            vec3 lightDir = normalize(lightPosVF[i] - fragPos);
            vec3 lightDirTBN = TBN * lightDir;

            // Phong or Blinn-Phong lighting model calculations
            float diff = max(dot(normal, lightDirTBN), 0.0);
            vec3 reflectDir = reflect(-lightDirTBN, normal);
            float spec = pow(max(dot(viewDirTBN, reflectDir), 0.0), 16.0);

            vec3 ambient = 0.8 * lightColor[i].rgb;
            vec3 diffuse = diff * lightColor[i].rgb;
            vec3 specular = spec * vec3(1.0);

            lighting += ambient + diffuse + specular;
        }

        vec3 albedo = texture(colorTex, uvCoord).rgb;
        fragColor = vec4(albedo * lighting, 1.0);
    }
}
