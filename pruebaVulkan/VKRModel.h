#pragma once
#include "VKRMaterial.h"
#include "VKRDebugMaterial.h"

#include <unordered_map>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;

		struct R_Mesh
		{// lo necesario para poder renderizar una Malla
		public:
			std::vector<Vertex3D> m_Vertices;
			std::vector<uint16_t> m_Indices;
			// Base_color, metallicRoughtness, normal Textures
			uint32_t m_Material;
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
			VkBuffer m_IndexBuffer;
			VkDeviceMemory m_IndexBufferMemory;
		public:
			R_Mesh() {}
			void Cleanup(VkDevice _LogicDevice)
			{
				if (m_Indices.size() > 0)
				{
					vkDestroyBuffer(_LogicDevice, m_IndexBuffer, nullptr);
					vkFreeMemory(_LogicDevice, m_IndexBufferMemory, nullptr);
				}
				vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
				vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
			}
		};
		struct R_Model //Render Model
		{// lo necesario para poder renderizar un Modelo
		public:
			R_Model() {}
		public:
			glm::vec3 m_Pos{ 0.0f, 1.0f, 0.0f };
			std::vector<R_Mesh*> m_Meshes;
			std::unordered_map<uint32_t, R_Material*> m_Materials;
			char m_Path[64];
		};

		struct R_DbgModel
		{
		public:
			R_DbgMaterial m_Material;
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
			glm::vec3 m_Pos{ 0.0f };
			glm::mat4 m_ModelMatrix = glm::mat4{ 1.0 };
			std::vector<DBG_Vertex3D> m_Vertices =
			{   // x     y     z
				{{-1.0, -1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 0
				{{-1.0, -1.0,  1.0}, {1.0f, 1.0f, 0.0f}}, // 1
				{{-1.0,  1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 2
			};
		public:
			void Cleanup(VkDevice _LogicDevice)
			{
				vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
				vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
				m_Material.Cleanup(_LogicDevice);
			}
		};
	}
}
