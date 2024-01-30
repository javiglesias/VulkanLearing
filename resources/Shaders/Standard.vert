#version 450

layout(binding=0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosition;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out vec3 normal;


void main() {
    gl_Position = ubo.model * ubo.projection * ubo.view  * vec4(inPosition, 1.0);
	fragColor = inColor;
    texCoord = aTexCoord;
	fragPosition = inPosition;
	normal = aNormal;
}
