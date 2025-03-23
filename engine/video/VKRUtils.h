#ifndef _C_UTILS
#define _C_UTILS
#define USE_VMA 0
#include "../core/VKRRenderPass.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <cstring>
#endif
#include <string>
#include <cstdio>
#include <cmath>

struct VmaAllocator_T;
typedef struct VmaAllocation_T* VmaAllocation;

inline
static void VK_ASSERT(bool _check)
{
	if (_check)
	{
		__debugbreak();
		exit(-69);
	}
}

enum CONSOLE_COLOR
{
	BLUE = 9,
	GREEN = 10,
	RED = 12,
	PURPLE = 13,
	YELLOW = 14,
	NORMAL = 15
};
#ifdef WIN32
inline HANDLE  hConsole;
#endif
inline
void ChangeColorConsole(CONSOLE_COLOR _color)
{
#ifdef WIN32
	if (!hConsole)
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, _color);
#endif
}

namespace VKR
{
	namespace render { extern std::string g_ConsoleMSG; }
	namespace utils
	{
		inline VmaAllocator_T* vma_allocator;
		void VMA_Initialize(VkPhysicalDevice _gpu, VkDevice _LogicDevice, VkInstance _instance);
		void VMA_CreateImage(VkMemoryPropertyFlags _memProperties, VkImageCreateInfo* _ImageCreateInfo
			, VkImage* Image_, VmaAllocation* Allocation_);
		void VMA_BindTextureMemory(VkImage _image, VmaAllocation _allocation);
		void VMA_DestroyImage(VkImage _image, VmaAllocation _allocation);
		struct GPUInfo
		{
			uint32_t minUniformBufferOffsetAlignment = 0;
			VkPhysicalDevice m_Device = VK_NULL_HANDLE;

			VkPhysicalDeviceProperties			   m_Properties{};
			VkPhysicalDeviceMemoryProperties	   m_MemProps{};
			VkPhysicalDeviceFeatures			   m_Features{};
			std::vector< VkExtensionProperties >   m_ExtensionsProps{};
			std::vector<const char*>               m_DeviceExtensions;
			std::vector< VkDeviceQueueCreateInfo> m_QueueCreateInfos;
			VkSurfaceCapabilitiesKHR		  m_SurfaceCaps{};
			std::vector< VkSurfaceFormatKHR > m_SurfaceFormats{};
			std::vector< VkPresentModeKHR >	  m_PresentModes{};

			std::vector< VkQueueFamilyProperties > m_QueueFamiliesProps{};
			const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
			/*VkPhysicalDeviceProperties2			   properties2{};
			VkPhysicalDeviceMaintenance3Properties properties3{};*/
			VkFormat FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features);

			VkFormat FindDepthTestFormat();
			bool CheckValidationLayerSupport();
			unsigned int FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties);
			void CheckValidDevice(VkInstance _Instance);
		};

		struct VKContext
		{
			GPUInfo  m_GpuInfo;
			VkDevice m_LogicDevice = VK_NULL_HANDLE;

			VKR::render::RenderPass* m_RenderPass;
			VKR::render::RenderPass* m_ShadowPass;

			unsigned int m_GraphicsComputeQueueFamilyIndex = 0;
			unsigned int m_TransferQueueFamilyIndex = 0;
			VkQueue m_GraphicsComputeQueue;
			VkQueue m_TransferQueue;
			VkQueue m_PresentQueue;

			//std::array< VkPipeline, SWAPCHAIN_BUFFERING_LEVEL > boundGraphicsPipelines{};
			void CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo);
			void CreateGeometryPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo);
			bool HasStencilComponent(VkFormat format);
			void CreateLogicalDevice();
			void CreateDevice(VkInstance _Instance);
			void CreateShadowRenderPass();
			void Cleanup();
		};
		inline VKContext g_context;
		inline VKContext& GetVKContext()
		{
			return g_context;
		}

		inline 
		void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_,
			VkDeviceMemory& bufferMem_)
		{
			auto renderContext = VKR::utils::GetVKContext();
			unsigned int queueFamilyIndices[] = { renderContext.m_GraphicsComputeQueueFamilyIndex, renderContext.m_TransferQueueFamilyIndex };
			VkBufferCreateInfo mBufferInfo{};
			mBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			mBufferInfo.size = _size;
			mBufferInfo.usage = _usage;
			mBufferInfo.sharingMode = _shareMode;
			mBufferInfo.pQueueFamilyIndices = queueFamilyIndices;
			mBufferInfo.queueFamilyIndexCount = 2;

			if (vkCreateBuffer(renderContext.m_LogicDevice, &mBufferInfo, nullptr, &buffer_) != VK_SUCCESS)
				exit(-6);
			VkMemoryRequirements mem_Requ;
			vkGetBufferMemoryRequirements(renderContext.m_LogicDevice, buffer_, &mem_Requ);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = mem_Requ.size;
			allocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(mem_Requ.memoryTypeBits, _memFlags);
			if (vkAllocateMemory(renderContext.m_LogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
				exit(-8);

			vkBindBufferMemory(renderContext.m_LogicDevice, buffer_, bufferMem_, 0);
		}

		inline 
		void CreateSSBOBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_,
			VkDeviceMemory& bufferMem_)
		{
			auto renderContext = VKR::utils::GetVKContext();
			unsigned int queueFamilyIndices[] = { renderContext.m_GraphicsComputeQueueFamilyIndex, renderContext.m_TransferQueueFamilyIndex };
			VkBufferCreateInfo mBufferInfo{};
			mBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			mBufferInfo.size = _size;
			mBufferInfo.usage = _usage;
			mBufferInfo.sharingMode = _shareMode;
			mBufferInfo.pQueueFamilyIndices = queueFamilyIndices;
			mBufferInfo.queueFamilyIndexCount = 2;

			if (vkCreateBuffer(renderContext.m_LogicDevice, &mBufferInfo, nullptr, &buffer_) != VK_SUCCESS)
				exit(-6);
			VkMemoryRequirements mem_Requ;
			vkGetBufferMemoryRequirements(renderContext.m_LogicDevice, buffer_, &mem_Requ);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = mem_Requ.size;
			allocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(mem_Requ.memoryTypeBits, _memFlags);
			if (vkAllocateMemory(renderContext.m_LogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
				exit(-8);

			vkBindBufferMemory(renderContext.m_LogicDevice, buffer_, bufferMem_, 0);
		}

		inline
		VkCommandBuffer BeginSingleTimeCommandBuffer(VkCommandPool _CommandPool)
		{
			auto renderContext = VKR::utils::GetVKContext();
			VkCommandBufferAllocateInfo mAllocInfo{};
			mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			mAllocInfo.commandPool = _CommandPool;
			mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			mAllocInfo.commandBufferCount = 1;
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(renderContext.m_LogicDevice, &mAllocInfo, &commandBuffer) != VK_SUCCESS)
				exit(-12);
			VkCommandBufferBeginInfo mBeginInfo{};
			mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			mBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			if (vkBeginCommandBuffer(commandBuffer, &mBeginInfo) != VK_SUCCESS)
				exit(-13);
			return commandBuffer;
		}

		inline
		void EndSingleTimeCommandBuffer(VkCommandBuffer _commandBuffer, VkCommandPool _CommandPool, VkQueue _queue)
		{
			auto renderContext = VKR::utils::GetVKContext();
			VK_ASSERT(vkEndCommandBuffer(_commandBuffer));
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &_commandBuffer;

			vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(_queue);
			vkFreeCommandBuffers(renderContext.m_LogicDevice, _CommandPool, 1, &_commandBuffer);
		}

		inline
		void CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size, VkCommandPool _CommandPool, VkQueue _queue)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
			// Copiar desde el Stagging buffer al buffer
			VkBufferCopy copyRegion{};
			copyRegion.size = _size;
			vkCmdCopyBuffer(commandBuffer, _src, dst_, 1, &copyRegion);
			EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, _queue);
		}
	}
}

#endif
