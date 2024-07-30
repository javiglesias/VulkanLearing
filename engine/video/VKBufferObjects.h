#ifndef _C_BUFFER_OBJECTS
#define _C_BUFFER_OBJECTS

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct UniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
	alignas(16) glm::vec3 cameraPosition;
};

struct DebugUniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};

struct ShadowUniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
	alignas(16) glm::mat4 depthMVP;
};

struct CubemapUniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};

struct DynamicBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::vec4 modelOpts; // 0: miplevel
	alignas(16) glm::vec4 addOpts; // 0: num current Lights
	alignas(16) glm::vec4 aligned[2];
};
struct LightBufferObject
{
	alignas(16) glm::mat4 View; // 64
	alignas(16) glm::mat4 Proj;
	alignas(16) glm::vec4 Position;
	alignas(16) glm::vec4 Color;
	alignas(16) glm::vec4 Opts; // 0: lightType,1: shadow 
	alignas(16) glm::vec4 addOpts; // Kc, Kl, Kq
	alignas(16) glm::vec4 aligned[4]; //16
};
#endif