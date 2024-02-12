#include <vulkan/vulkan_core.h>
#include "VKRMaterial.h"
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
	R_Mesh(){}
	void Cleanup(VkDevice _LogicDevice)
	{
		if(m_Indices.size() > 0)
		{
			vkDestroyBuffer(_LogicDevice, m_IndexBuffer, nullptr);
			vkFreeMemory(_LogicDevice,m_IndexBufferMemory, nullptr);
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
	std::vector<R_Mesh*> m_Meshes;
	char m_Path[64];
};