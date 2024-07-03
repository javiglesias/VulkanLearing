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

layout(set=0, binding=6) uniform LightBufferObject
{
	mat4 lightView;
	mat4 lightProj;
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightOpts; // 0: lightType, 1: shadow 
	vec4 additionalLightOpts; // Kc, Kl, Kq
} libO[4];

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1)  out vec2 texCoord;
layout(location = 2)  out vec3 normal;
layout(location = 3)  out vec3 lightPosition;
layout(location = 4)  out vec3 viewerPosition;
layout(location = 5)  out vec3 lightColor;
layout(location = 6)  out vec4 shadowCoord;
layout(location = 7)  out float shadowBias;
layout(location = 8)  out float projectShadow;
layout(location = 9)  out float mipLevel;
layout(location = 10) out vec4 pointLightConstants;


void main() {
    gl_Position = ubo.projection * ubo.view * dynO.model * vec4(inPosition, 1.0);
    texCoord = aTexCoord;
	fragPosition = inPosition;
	normal = mat3(transpose(inverse(dynO.model))) * aNormal;
    lightPosition = libO[0].lightPosition.xyz;
    viewerPosition = ubo.viewerPosition;
    lightColor = libO[0].lightColor.xyz;
	shadowCoord = libO[0].lightProj * libO[0].lightView * dynO.model * vec4(inPosition, 1.0);
	shadowBias = libO[0].lightOpts.y;
	projectShadow = libO[0].lightOpts.x;
	mipLevel = dynO.modelOpts.x;
	pointLightConstants = libO[0].additionalLightOpts;
}