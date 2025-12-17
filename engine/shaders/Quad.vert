#version 460

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 aTexCoord;

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
} ubo;

layout(set=0, binding=1) uniform DynamicBufferObject
{
    mat4 model;
	vec4 lightOpt; // 0: Bias
} dynO;

void main() 
{
	gl_Position = vec4(inPos.xy, 0.0, 1.0);
}