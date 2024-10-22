#version 460

layout(location = 0) in vec3 inPosition;

layout(set=0, binding=0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
} ubo;

const vec3 mPos[4] = vec3[4] (
	vec3(-1.0, 0.0, -1.0),
	vec3( 1.0, 0.0, -1.0),
	vec3( 1.0, 0.0,  1.0),
	vec3(-1.0, 0.0,  1.0)
);
const int mIndices[6] = int[6](0, 2, 1, 2, 0, 3);

void main() 
{
	int Idx = mIndices[gl_VertexIndex];
	vec4 vPos = vec4(mPos[Idx], 1.0);
    gl_Position = ubo.view * ubo.projection * vPos;
}