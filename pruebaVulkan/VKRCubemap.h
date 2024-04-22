#pragma once
#include "VKRCubemapMaterial.h"
#include "Types.h"

namespace VKR
{
	namespace render
	{
		class R_Cubemap
		{
		public: // variables
			char m_Path[64];
			R_CubemapMaterial* m_Material;
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
			glm::vec3 m_Pos{ 0.0f };
			glm::vec3 m_Rotation{ 0.0f };
			glm::mat4 m_ModelMatrix = glm::mat4{ 1.0 };
			std::vector<DBG_Vertex3D> m_Vertices;
		public: // functions
			R_Cubemap(std::string _texturePath);
		};
	}
}