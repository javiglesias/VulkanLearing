#ifndef RENDERABLE_H
#define RENDERABLE_H
#include "Types.h"
#include "../core/Materials/VKRMaterial.h"

namespace VKR
{
    namespace render
    {
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
			VkBuffer m_VertexBuffer = nullptr;
			VkDeviceMemory m_VertexBufferMemory = nullptr;
			VkBuffer m_IndexBuffer = nullptr;
			VkDeviceMemory m_IndexBufferMemory = nullptr;
			PBR_material metallic_roughness;
			VKRenderable();
			void Cleanup(VkDevice _LogicDevice);
        };
    }
}
#endif