#version 330 core

layout (location = 0) in vec2 pos;  /*  Positions of the vertices on the rendering quad */

out vec2 uvCoord;

void main(void)
{
    /*  Hard-coded texture coordinate to map 4 corners of the quad to 
        4 corners of the texture.
    */
    const vec2 uv[4]    = vec2[4](
                                  vec2(0.0f, 0.0f),
                                  vec2(1.0f, 0.0f),
                                  vec2(0.0f, 1.0f),
                                  vec2(1.0f, 1.0f)
                                  );

    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    uvCoord = uv[gl_VertexID];
}
