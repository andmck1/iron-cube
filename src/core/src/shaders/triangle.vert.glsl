#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    // Convert NDC to clip space (z=0, w=1)
    gl_Position = vec4(inPos, 0.0, 1.0);
    fragColor = inColor;
}
