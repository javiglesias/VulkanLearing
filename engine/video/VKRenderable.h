#ifndef RENDERABLE_H
#define RENDERABLE_H
#include "Types.h"
#include "../core/Materials/VKRMaterial.h"

namespace VKR
{
    namespace render
    {
		inline const int FRAMES_IN_FLIGHT = 2;
        struct VKRenderable
        {
			std::vector<Vertex3D> m_Vertices;
			std::vector<uint16_t> m_Indices;
			glm::mat4 m_ModelMatrix = glm::mat4(1.f);
			glm::vec4 base_color { 0.0f };
			glm::vec3 m_Pos { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_Rotation { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_Scale { 1.0f, 1.0f, 1.0f };
			// Base_color, metallicRoughtness, normal Textures
			uint32_t m_Material;
			VkBuffer m_VertexBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_VertexBufferMemory[FRAMES_IN_FLIGHT];
			VkBuffer m_IndexBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_IndexBufferMemory[FRAMES_IN_FLIGHT];
			VkBuffer m_ComputeBuffer[FRAMES_IN_FLIGHT];
			VkDeviceMemory m_ComputeBufferMemory[FRAMES_IN_FLIGHT];
			PBR_material metallic_roughness;
			VKRenderable();
			void Cleanup(VkDevice _LogicDevice);
        };
    }
}
#endif