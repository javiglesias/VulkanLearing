#include "VKRenderable.h"
#include "../core/Materials/VKRTexture.h"
namespace VKR
{
	namespace render
	{
		VKRenderable::VKRenderable()
		{
			memset(m_Id, 0, 256);
		}
		void VKRenderable::Cleanup(VkDevice _LogicDevice)
		{
			for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				if (m_Indices.size() > 0)
				{
					vkDestroyBuffer(_LogicDevice, m_IndexBuffer[i], nullptr);
					vkFreeMemory(_LogicDevice, m_IndexBufferMemory[i], nullptr);
				}
				vkDestroyBuffer(_LogicDevice, m_VertexBuffer[i], nullptr);
				vkFreeMemory(_LogicDevice, m_VertexBufferMemory[i], nullptr);

				vkDestroyBuffer(_LogicDevice, m_ComputeBuffer[i], nullptr);
				vkFreeMemory(_LogicDevice, m_ComputeBufferMemory[i], nullptr);
			}
		}
	}
}