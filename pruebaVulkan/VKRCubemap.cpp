#include "VKRCubemap.h"
#include "DebugModels.h"

namespace VKR
{
	namespace render
	{
		R_Cubemap::R_Cubemap(std::string _texturePath)
		{
			fprintf(stdout, "Loading cubemap %s\n", _texturePath.c_str());
			strcpy(m_Path, _texturePath.c_str());	
			m_Material = new R_CubemapMaterial(_texturePath);
			// TODO creacion de los vertices
			m_Vertices = m_CubeVertices;
		}
		void R_Cubemap::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
			vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
			m_Material->Cleanup(_LogicDevice);
		}
	}
}