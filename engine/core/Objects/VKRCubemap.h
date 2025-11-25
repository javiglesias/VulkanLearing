#ifndef _C_CUBEMAP
#define _C_CUBEMAP

#include "../Materials/VKRCubemapMaterial.h"
#include "../../video/Types.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#define MAX_FRAMES 2

namespace VKR
{
	namespace render
	{
		struct R_Model;
		class R_Cubemap
		{
		public: // variables
			char m_Path[64];
			R_Model* m_gltf;
			Texture* m_Texture;
			R_CubemapMaterial* m_Material;
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
			glm::vec3 m_Pos{ 0.0f };
			glm::vec3 m_Rotation{ 0.0f };
			glm::mat4 m_ModelMatrix = glm::mat4{ 1.0 };
			std::vector<DBG_Vertex3D> m_Vertices;
			std::array<VkBuffer, MAX_FRAMES> uniform_Buffers;
			std::array<VkDeviceMemory, MAX_FRAMES> uniform_Buffers_Memory;
			std::array<void*, MAX_FRAMES> uniforms_Buffers_Mapped;
		public: // functions
			R_Cubemap(const char* _texturePath);
			void Prepare(VKBackend* _backend);
			void GenerateBuffers();
			void Cleanup(VkDevice _LogicDevice);
			void Draw(VKBackend* _backend, int _CurrentFrame);
		};
	}
}
#endif