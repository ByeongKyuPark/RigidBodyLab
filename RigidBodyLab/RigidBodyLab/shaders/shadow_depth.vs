#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 lightSpaceMat;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMat* vec4(position, 1.0);
}
