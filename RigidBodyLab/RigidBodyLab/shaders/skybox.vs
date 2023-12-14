#version 330 core

uniform mat4 viewMat;

out vec3 texCoord;

void main(void)
{
    /*  Render background using a full-screen quad defined as the far plane in NDC frame.
        This quad texture coordinates are defined in world-frame as the 4 back corners 
        of a 2x2x2 box symmetric about the world origin.
    */
    vec2 vertices[4];
    vertices[0] = vec2(-1.0, -1.0);
    vertices[1] = vec2(1.0, -1.0);
    vertices[2] = vec2(-1.0, 1.0);
    vertices[3] = vec2(1.0, 1.0);
     
     // Transform the quad's texture coordinates with the transpose of the view matrix
    texCoord = transpose(mat3(viewMat)) * vec3(vertices[gl_VertexID], -1.0);

    // Set the position of the vertex using the quad vertices
    gl_Position = vec4(vertices[gl_VertexID], 1.0, 1.0);
}