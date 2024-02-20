#version 450

layout(binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
} ubo;

layout(set=0, binding=4) uniform DynamicBufferObject
{
    mat4 model;
} dynO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main()
{
    gl_Position = dynO.model * ubo.projection * ubo.view  * vec4(inPosition, 1.0);
    outColor = inColor;
}
