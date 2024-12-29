#version 460

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
    vec3 viewerPosition;
} ubo;

layout(set=0, binding=2) uniform DynamicBufferObject
{
    mat4 model;
    vec4 modelOpts; // x: depth, y:MipLevel, z: specular, w: normals
	vec4 addOpts; // x: num current Lights
} dynO;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 viewerPosition;
layout(location = 4) out vec4 shadowCoord;
layout(location = 5) out float mipLevel;
layout(location = 6) out float nLights;
layout(location = 7) out float renderDepth;
layout(location = 8) out float renderSpecular;
layout(location = 9) out float renderNormals;

void main() {
    gl_Position = ubo.projection * ubo.view * dynO.model * vec4(inPosition, 1.0);
    texCoord = aTexCoord;
	fragPosition = vec3(dynO.model * vec4(inPosition, 1.0));
	normal = mat3(transpose(inverse(dynO.model))) * aNormal;
    viewerPosition = ubo.viewerPosition;
	shadowCoord = dynO.model * vec4(inPosition, 1.0);
    
	renderDepth = dynO.modelOpts.x;
	mipLevel    = dynO.modelOpts.y;
    renderSpecular = dynO.modelOpts.z;
    renderNormals  = dynO.modelOpts.w;
	
	nLights 	   = dynO.addOpts.x;
}
