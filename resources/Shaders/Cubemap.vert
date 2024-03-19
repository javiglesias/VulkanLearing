#version 450

layout (set=0, location = 0) in vec3 inPosition;
layout (set=0, location = 1) in vec2 aTexCoord;

layout(location = 0) out vec2 texCoord;


void main() 
{
	gl_Position = vec4(inPosition, 1.0);
	texCoord = aTexCoord;
}
