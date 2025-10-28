#version 450

layout(location = 0)  in vec3 inColor;
layout (location = 1) in vec3 inUVW;

layout(location = 0)  out vec4 outColor;

layout (set=0, binding=1) uniform sampler2D samplerCubeMap;

void main() 
{
    outColor = texture(samplerCubeMap, inUVW.xy);
}