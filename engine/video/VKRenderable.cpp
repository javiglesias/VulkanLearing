#include "VKRenderable.h"
#include "../core/Materials/VKRTexture.h"
namespace VKR
{
	namespace render
	{
		VKRenderable::VKRenderable()
		{
		}
		void VKRenderable::Cleanup(VkDevice _LogicDevice)
		{
			if (m_Indices.size() > 0)
			{
				vkDestroyBuffer(_LogicDevice, m_IndexBuffer, nullptr);
				vkFreeMemory(_LogicDevice, m_IndexBufferMemory, nullptr);
			}
			vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
			vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
		}
	}
}