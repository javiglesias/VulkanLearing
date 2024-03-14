#version 450

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
} ubo;

layout(set=0, binding=1) uniform DynamicBufferObject
{
    mat4 model;
} dynO;

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;


void main() {
    gl_Position = dynO.model * ubo.projection * ubo.view  * vec4(inPosition, 1.0);
}