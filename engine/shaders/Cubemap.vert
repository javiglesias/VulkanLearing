#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
} ubo;

layout(set=0, binding=2) uniform DynamicBufferObject
{
    mat4 model;
	vec4 lightOpt; // 0: Bias
} dynO;

layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = inPos;
	// Convert cubemap coordinates into Vulkan coordinate space
	outUVW.xy *= -1.0;
	// Remove translation from view matrix
	mat4 viewMat = mat4(mat3(dynO.model));
	gl_Position = ubo.projection * viewMat * vec4(inPos.xyz, 1.0);
	 //gl_Position = ubo.projection * ubo.view * dynO.model * vec4(inPos, 1.0);
}