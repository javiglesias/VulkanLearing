#version 450

layout(binding=0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewerPosition;
    vec3 lightPosition;
    vec3 lightColor;
} ubo;

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 lightPosition;
layout(location = 4) out vec3 viewerPosition;
layout(location = 5) out vec3 lightColor;


void main() {
    gl_Position = ubo.model * ubo.projection * ubo.view  * vec4(inPosition, 1.0);
    texCoord = aTexCoord;
	fragPosition = inPosition;
	normal = aNormal;
    lightPosition = ubo.lightPosition;
    viewerPosition = ubo.viewerPosition;
    lightColor = ubo.lightColor;
}
