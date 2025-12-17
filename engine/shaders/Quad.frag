#version 460

layout (set=0, binding=2) uniform sampler2D samplerCubeMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = vec4(1.0);
}