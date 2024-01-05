#version 330 core
out vec4 fragColor;

void main()
{
    //don't need to explicitly write anything to the output color
    //The depth value (z-value of gl_FragCoord) is automatically written to the depth buffer by OpenGL.
    fragColor=vec4(1.f,0.f,0.f,1.f);
}
