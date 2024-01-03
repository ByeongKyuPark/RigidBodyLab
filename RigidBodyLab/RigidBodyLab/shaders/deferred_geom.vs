#version 330 core
// Uniforms and layout locations
uniform mat4 mvMat;  // Model-view matrix
uniform mat4 nmvMat; // Normal model-view matrix
uniform mat4 projMat; // Projection matrix
uniform int objType; 

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nrm;
layout(location = 2) in vec3 tan;
layout(location = 4) in vec2 uv; // Include UV coordinates

out vec3 fragPos;    // Fragment position in view space
out vec3 fragNormal; // Fragment normal in view space
out vec3 fragTan;    // Fragment tangent in view space
out vec2 fragUV;     // Fragment UV coordinates
out float fragObjType;    

void main() {
    vec4 viewPos = mvMat * vec4(pos, 1.0);
    fragPos = viewPos.xyz;
    fragNormal = normalize(mat3(nmvMat) * nrm);
    fragTan = normalize(mat3(mvMat) * tan);
    fragUV = uv; // Pass UV coordinates to fragment shader

    // convert the received objectType from `int` to `float` and then store it in the alpha channel of the tangent.
    // this way, all object types other than normal mapped plane(=0) will have this value clamped to 1.0f
    fragObjType = objType; // Convert int to float for passing to FS

    gl_Position = projMat * viewPos;
}
