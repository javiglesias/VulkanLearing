#include "VKRUtils.h"

#include <signal.h>

namespace VKR
{
	namespace utils
	{
#if 0
		void GenerateDynamicAndUniformBuffers()
		{
#pragma region RENDER_BUFFERS
			VkDeviceSize bufferSize = sizeof(UniformBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_UniformBuffers[i], m_UniformBuffersMemory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, m_UniformBuffersMemory[i], 0,
					bufferSize, 0, &m_UniformsBuffersMapped[i]);
			}
			auto DynAlign = sizeof(DynamicBufferObject);
			DynAlign = (DynAlign + utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			VkDeviceSize checkBufferSize = m_Meshes.size() * DynAlign;
			VkDeviceSize dynBufferSize = m_Meshes.size() * sizeof(DynamicBufferObject);
			if (dynBufferSize != checkBufferSize)
#ifdef _MSVC
				__debugbreak();
#else
				raise(SIGTRAP);
#endif
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				utils::CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DynamicBuffers[i], m_DynamicBuffersMemory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, m_DynamicBuffersMemory[i], 0,
					dynBufferSize, 0, &m_DynamicBuffersMapped[i]);
			}
#pragma endregion
		}
#endif
	}
}
