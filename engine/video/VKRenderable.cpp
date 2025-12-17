#include "VKRenderable.h"
#include "../core/Materials/VKRTexture.h"
#include <cstring>

#include "VKBufferObjects.h"

namespace VKR
{
	namespace render
	{
		VKRenderable3D::VKRenderable3D()
		{
			memset(m_Id, 0, 256);
		}
		void VKRenderable3D::Cleanup(VkDevice _LogicDevice)
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

		VKRenderable2D::VKRenderable2D()
		{
			memset(m_Id, 0, 256);
			material = new render::R_Material2D();
		}

		void VKRenderable2D::Cleanup(VkDevice _LogicDevice)
		{
			for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				vkDestroyBuffer(_LogicDevice, m_VertexBuffer[i], nullptr);
				vkFreeMemory(_LogicDevice, m_VertexBufferMemory[i], nullptr);

				vkDestroyBuffer(_LogicDevice, m_ComputeBuffer[i], nullptr);
				vkFreeMemory(_LogicDevice, m_ComputeBufferMemory[i], nullptr);
			}
		}
	}
}
