#version 450

layout (binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 tint;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outTint;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    outUV = inUV;
    outTint = ubo.tint;
}