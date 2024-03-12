#version 450
layout(set=0, binding=0) uniform sampler2D inTexture;

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(vec3(texture(inAmbientTexture, texCoord)), 1.0);
}
