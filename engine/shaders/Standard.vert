#version 460

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
    vec3 viewerPosition;
} ubo;

layout(set=0, binding=4) uniform DynamicBufferObject
{
    mat4 model;
    vec4 modelOpts; // 0: MipLevel
} dynO;

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 viewerPosition;
layout(location = 4) out vec4 shadowCoord;
layout(location = 5) out float mipLevel;

void main() {
    gl_Position = ubo.projection * ubo.view * dynO.model * vec4(inPosition, 1.0);
    texCoord = aTexCoord;
	fragPosition = inPosition;
	normal = mat3(transpose(inverse(dynO.model))) * aNormal;
    viewerPosition = ubo.viewerPosition;
	mipLevel = dynO.modelOpts.x;
	shadowCoord = dynO.model * vec4(inPosition, 1.0);
}