#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gColor;

void main() {
    vec3 color = texture(gColor, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
