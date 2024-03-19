#version 450

layout(set=0, binding=2) uniform sampler2D inDiffuseTexture;
layout(set=0, binding=3) uniform sampler2D inSpecularTexture;
layout(set=0, binding=4) uniform sampler2D inAmbientTexture;

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(vec3(texture(inDiffuseTexture, texCoord)), 1.0);
}
