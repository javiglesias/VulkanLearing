#ifndef RENDERABLE_H
#define RENDERABLE_H
#include "Types.h"
#include "../core/Materials/VKRMaterial.h"
#define FRAMES_IN_FLIGHT 2
namespace VKR
{
	namespace render
	{
		struct VKRenderable3D
		{
			char m_Id[256];
			std::vector<Vertex3D> m_Vertices;
			std::vector<uint16_t> m_Indices;
			glm::mat4 m_ModelMatrix = glm::mat4(1.f);
			glm::vec4 base_color { 0.0f };
			glm::vec3 m_Pos { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_Rotation { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_Scale { 1.0f, 1.0f, 1.0f };
			// Base_color, metallicRoughtness, normal Textures
			uint32_t m_Material = -1;
			VkBuffer m_VertexBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_VertexBufferMemory[FRAMES_IN_FLIGHT];
			VkBuffer m_IndexBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_IndexBufferMemory[FRAMES_IN_FLIGHT];
			VkBuffer m_ComputeBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_ComputeBufferMemory[FRAMES_IN_FLIGHT];
			PBR_material metallic_roughness;
			VKRenderable3D();
			void Cleanup(VkDevice _LogicDevice);
		};
		// 2D 
		struct VKRenderable2D
		{
			char m_Id[256];
			std::vector<Vertex2D> m_Vertices;
			//std::vector<uint16_t> m_Indices;

			glm::mat4 m_ModelMatrix = glm::mat4(1.f);
			glm::vec4 base_color{ 0.0f };
			glm::vec2 m_Pos{ 0.0f, 1.0f};
			glm::vec2 m_Rotation{ 0.0f, 1.0f};

			R_Material2D* material;

			// Vulkan buffers
			VkBuffer m_VertexBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_VertexBufferMemory[FRAMES_IN_FLIGHT];
			VkBuffer m_IndexBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_IndexBufferMemory[FRAMES_IN_FLIGHT];
			VkBuffer m_ComputeBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_ComputeBufferMemory[FRAMES_IN_FLIGHT];
		// funtions
			VKRenderable2D();
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
#endif