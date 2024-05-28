#version 460

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
    mat4 lightView;
    mat4 lightProj;
    vec3 viewerPosition;
    vec3 lightPosition;
    vec3 lightColor;
} ubo;

layout(set=0, binding=4) uniform DynamicBufferObject
{
    mat4 model;
    vec4 lightOpt; // 0: Bias 1: projectShadow
} dynO;

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 lightPosition;
layout(location = 4) out vec3 viewerPosition;
layout(location = 5) out vec3 lightColor;
layout(location = 6) out vec4 shadowCoord;
layout(location = 7) out float shadowBias;
layout(location = 8) out float projectShadow;


void main() {
    gl_Position = ubo.projection * ubo.view * dynO.model * vec4(inPosition, 1.0);
    texCoord = aTexCoord;
	fragPosition = inPosition;
	normal = mat3(transpose(inverse(dynO.model))) * aNormal;
    lightPosition = ubo.lightPosition;
    viewerPosition = ubo.viewerPosition;
    lightColor = ubo.lightColor;
	shadowCoord = ubo.lightProj * ubo.lightView * dynO.model * vec4(inPosition, 1.0);
	shadowBias = dynO.lightOpt.x;
	projectShadow = dynO.lightOpt.y;
}