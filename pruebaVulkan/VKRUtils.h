#pragma once
#include "VKRRenderPass.h"
#include <vector>
namespace VKR
{
	namespace render
	{
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
			unsigned int m_GraphicsQueueFamilyIndex = 0;
			unsigned int m_TransferQueueFamilyIndex = 0;
			VkQueue m_GraphicsQueue;
			VkQueue m_TransferQueue;
			VkQueue m_PresentQueue;

			//std::array< VkPipeline, SWAPCHAIN_BUFFERING_LEVEL > boundGraphicsPipelines{};
			void CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo);
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

		inline VkFormat GPUInfo::FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features)
		{
			for (VkFormat format : _candidates)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(m_Device, format, &props);
				if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
					return format;
				if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
					return format;
			}
			//g_ConsoleMSG += "No found supported format\n";
			printf("No found supported format\n");
			exit(-79);
		}

		inline VkFormat GPUInfo::FindDepthTestFormat()
		{
			auto candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
			auto tiling = VK_IMAGE_TILING_OPTIMAL;
			auto features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
			return FindSupportedFormat(candidates, tiling, features);
		}

		inline bool GPUInfo::CheckValidationLayerSupport()
		{
			unsigned int layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
			for (const char* layerName : m_ValidationLayers)
			{
				bool layerFound = false;
				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						return true;
					}
				}
			}

			return false;
		}

		inline unsigned int GPUInfo::FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties)
		{
			for (unsigned int i = 0; i < m_MemProps.memoryTypeCount; i++)
			{
				if (typeFilter & (1 << i) && (m_MemProps.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}
			printf("Not Found Memory Type %d", properties);
			exit(-9);
		}

		inline void GPUInfo::CheckValidDevice(VkInstance _Instance)
		{
			/// Asociamos el VkDevice a la GPU
			unsigned int deviceCount = 0;
			vkEnumeratePhysicalDevices(_Instance, &deviceCount, nullptr);
			if (deviceCount == 0)
			{
				fprintf(stderr, "Not GPU FOUND\n");
				exit(-1);
			}
			// Comprobamos que los Physical device cumplan lo necesario
			std::vector<VkPhysicalDevice> m_PhysicalDevices(deviceCount);
			vkEnumeratePhysicalDevices(_Instance, &deviceCount, m_PhysicalDevices.data());
			VkPhysicalDeviceProperties* deviceProp = new VkPhysicalDeviceProperties[deviceCount];
			VkPhysicalDeviceFeatures* deviceFeatures = new VkPhysicalDeviceFeatures[deviceCount];
			unsigned int m_GPUSelected = 0;
			for (int it = 0; it < deviceCount; it++)
			{
				vkGetPhysicalDeviceProperties(m_PhysicalDevices[it], &deviceProp[it]);
				if (strstr(deviceProp[it].deviceName, "NVIDIA") || strstr(deviceProp[it].deviceName, "AMD"))
				{
					m_GPUSelected = it;
					char tmp[512];
					sprintf(tmp, "\nGPU %d: %s\n", it, deviceProp[m_GPUSelected].deviceName);
					//g_ConsoleMSG += tmp;
					g_context.m_GpuInfo.minUniformBufferOffsetAlignment = deviceProp[m_GPUSelected].limits.minUniformBufferOffsetAlignment;
				}
				vkGetPhysicalDeviceFeatures(m_PhysicalDevices[it], &deviceFeatures[it]);
			}
			m_Device = m_PhysicalDevices[m_GPUSelected];
			vkGetPhysicalDeviceProperties(m_Device, &m_Properties);
			vkGetPhysicalDeviceFeatures(m_Device, &m_Features);
			vkGetPhysicalDeviceMemoryProperties(m_Device, &m_MemProps);
		}

		// VK CONTEXT
		inline void VKContext::CreateLogicalDevice()
		{
			VkDeviceCreateInfo m_CreateInfo{};
			m_CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			m_CreateInfo.pQueueCreateInfos = g_context.m_GpuInfo.m_QueueCreateInfos.data();
			m_CreateInfo.queueCreateInfoCount = g_context.m_GpuInfo.m_QueueCreateInfos.size();
			m_CreateInfo.pEnabledFeatures = &m_GpuInfo.m_Features;
			m_CreateInfo.enabledExtensionCount = g_context.m_GpuInfo.m_DeviceExtensions.size();
			m_CreateInfo.ppEnabledExtensionNames = g_context.m_GpuInfo.m_DeviceExtensions.data();
			m_CreateInfo.enabledLayerCount = 0;

			if (vkCreateDevice(m_GpuInfo.m_Device, &m_CreateInfo, nullptr, &g_context.m_LogicDevice) !=
				VK_SUCCESS)
			{
				printf("Failed to create Logical Device");
				exit(-1);
			}
		}
		inline void VKContext::CreateDevice(VkInstance _Instance)
		{
			m_GpuInfo.CheckValidDevice(_Instance);
			/// Comprobamos que tengamos la extension VK_KHR_swapchain soportada en el device GPU
			unsigned int deviceExtensionCount;
			vkEnumerateDeviceExtensionProperties(m_GpuInfo.m_Device, nullptr, &deviceExtensionCount, nullptr);
			std::vector<VkExtensionProperties> deviceAvailableExtensions(deviceExtensionCount);
			vkEnumerateDeviceExtensionProperties(m_GpuInfo.m_Device, nullptr, &deviceExtensionCount, deviceAvailableExtensions.data());
			for (const auto& ext : deviceAvailableExtensions)
			{
				// VK_KHR_buffer_device_address and VK_EXT_buffer_device_address not at the same time
				if (strcmp(ext.extensionName, "VK_EXT_buffer_device_address") == 0)
					continue;
				g_context.m_GpuInfo.m_DeviceExtensions.push_back(ext.extensionName);
			}
			/// Buscamos las familias de Colas.
			unsigned int queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(m_GpuInfo.m_Device, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(m_GpuInfo.m_Device, &queueFamilyCount, queueFamilies.data());
			// VK_QUEUE_GRAPHICS_BIT
			bool searchingGraphics = true, searchingTransfer = true;
			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					char tmp[256];
					sprintf(tmp, "Graphics Family: %d\n", m_TransferQueueFamilyIndex);
					/*g_ConsoleMSG += tmp;*/
					searchingGraphics = false;
				}
				if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					char tmp[256];
					sprintf(tmp, "Transfer Family: %d\n", m_TransferQueueFamilyIndex);
					//g_ConsoleMSG += tmp;
					searchingTransfer = false;
				}

				m_GraphicsQueueFamilyIndex += searchingGraphics;
				m_TransferQueueFamilyIndex += searchingTransfer;
				if (!searchingGraphics && !searchingTransfer)
					break;
			}

			/// Ahora vamos a crear el device logico para interactuar con él
			float queuePriority = 1.f;

			g_context.m_GpuInfo.m_QueueCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = m_GraphicsQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
				});

			g_context.m_GpuInfo.m_QueueCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = m_TransferQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
				});

			// Create logical device associated to physical device
			CreateLogicalDevice();
			vkGetDeviceQueue(m_LogicDevice, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_LogicDevice, m_TransferQueueFamilyIndex, 0, &m_TransferQueue);
		}

		inline void VKContext::CreateShadowRenderPass()
		{
			m_ShadowPass = new VKR::render::RenderPass();
			m_ShadowPass->CreateDepthAttachment(m_GpuInfo.FindDepthTestFormat(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			m_ShadowPass->CreateDepthOnlySubPass();
			m_ShadowPass->CreateRenderPass(m_LogicDevice);
		}

		inline void VKContext::Cleanup()
		{
			vkDeviceWaitIdle(m_LogicDevice);
			m_ShadowPass->Cleanup(m_LogicDevice);
			m_RenderPass->Cleanup(m_LogicDevice);
		}

		inline void VKContext::CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo)
		{

			m_RenderPass = new VKR::render::RenderPass();
			m_RenderPass->CreateColorAttachment(m_SwapChainCreateInfo->imageFormat);
			m_RenderPass->CreateDepthAttachment(m_GpuInfo.FindDepthTestFormat(),
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			m_RenderPass->CreateSubPass();
			m_RenderPass->CreateRenderPass(m_LogicDevice);
		}

		inline bool VKContext::HasStencilComponent(VkFormat format)
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}
	}
}

inline 
static void VK_ASSERT(bool _check)
{
	if (_check) exit(-69);
}

inline 
void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_,
	VkDeviceMemory& bufferMem_)
{
	auto renderContext = VKR::render::GetVKContext();
	unsigned int queueFamilyIndices[] = { renderContext.m_GraphicsQueueFamilyIndex, renderContext.m_TransferQueueFamilyIndex };
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
	auto rederContext = VKR::render::GetVKContext();
	allocInfo.memoryTypeIndex = rederContext.m_GpuInfo.FindMemoryType(mem_Requ.memoryTypeBits, _memFlags);
	if (vkAllocateMemory(renderContext.m_LogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
		exit(-8);

	vkBindBufferMemory(renderContext.m_LogicDevice, buffer_, bufferMem_, 0);
}

inline
void CreateImage(unsigned int _Width, unsigned int _Height, VkFormat _format, VkImageTiling _tiling,
	VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties,
	VkImage* _image, VkDeviceMemory* _imageMem)
{
	auto renderContext = VKR::render::GetVKContext();
	VkImageCreateInfo tImageInfo{};
	tImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	tImageInfo.imageType = VK_IMAGE_TYPE_2D;
	tImageInfo.extent.width = static_cast<uint32_t>(_Width);
	tImageInfo.extent.height = static_cast<uint32_t>(_Height);
	tImageInfo.extent.depth = 1;
	tImageInfo.mipLevels = 1;
	tImageInfo.arrayLayers = 1;
	tImageInfo.format = _format;
	tImageInfo.tiling = _tiling;
	tImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	tImageInfo.usage = _usage;
	tImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	tImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	tImageInfo.flags = 0;
	VK_ASSERT(vkCreateImage(renderContext.m_LogicDevice, &tImageInfo, nullptr, _image));
	VkMemoryRequirements memRequ;
	vkGetImageMemoryRequirements(renderContext.m_LogicDevice, *_image, &memRequ);
	VkMemoryAllocateInfo tAllocInfo{};
	tAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	tAllocInfo.allocationSize = memRequ.size;
	tAllocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(memRequ.memoryTypeBits, _memProperties);
	VK_ASSERT(vkAllocateMemory(renderContext.m_LogicDevice, &tAllocInfo, nullptr, _imageMem));
}

inline
VkCommandBuffer BeginSingleTimeCommandBuffer(VkCommandPool _CommandPool)
{
	auto renderContext = VKR::render::GetVKContext();
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
void EndSingleTimeCommandBuffer(VkCommandBuffer _commandBuffer, VkCommandPool _CommandPool)
{
	auto renderContext = VKR::render::GetVKContext();
	VK_ASSERT(vkEndCommandBuffer(_commandBuffer));
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;

	vkQueueSubmit(renderContext.m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(renderContext.m_GraphicsQueue);
	vkFreeCommandBuffers(renderContext.m_LogicDevice, _CommandPool, 1, &_commandBuffer);

}

inline
void CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size, VkCommandPool _CommandPool)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
	// Copiar desde el Stagging buffer al buffer
	VkBufferCopy copyRegion{};
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _src, dst_, 1, &copyRegion);
	EndSingleTimeCommandBuffer(commandBuffer, _CommandPool);
}

inline
void TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _old, VkImageLayout _new, VkCommandPool _CommandPool)
{
	auto renderContext = VKR::render::GetVKContext();
	VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
	VkImageMemoryBarrier iBarrier{};
	iBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	iBarrier.oldLayout = _old;
	iBarrier.newLayout = _new;
	iBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	iBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	iBarrier.image = _image;
	if (_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (renderContext.HasStencilComponent(_format))
		{
			iBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
		iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	iBarrier.subresourceRange.baseMipLevel = 0;
	iBarrier.subresourceRange.levelCount = 1;
	iBarrier.subresourceRange.baseArrayLayer = 0;
	iBarrier.subresourceRange.layerCount = 1;
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (_old == VK_IMAGE_LAYOUT_UNDEFINED && _new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		iBarrier.srcAccessMask = 0;
		iBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (_old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		iBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (_old == VK_IMAGE_LAYOUT_UNDEFINED && _new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		iBarrier.srcAccessMask = 0;
		iBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		printf("unsupported layout transition!");
		exit(-96);
	}
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &iBarrier);
	EndSingleTimeCommandBuffer(commandBuffer, _CommandPool);
}

inline 
void CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _w, uint32_t _h, VkDeviceSize _bufferOffset, VkCommandPool _CommandPool)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
	VkBufferImageCopy region{};
	region.bufferOffset = _bufferOffset;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { _w, _h, 1 };
	vkCmdCopyBufferToImage(commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	EndSingleTimeCommandBuffer(commandBuffer, _CommandPool);
}

inline 
VkImageView CreateImageView(VkImage _tImage, VkFormat _format, VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT)
{
	auto renderContext = VKR::render::GetVKContext();
	VkImageView tImageView;
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = _tImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = _format;
	viewInfo.subresourceRange.aspectMask = _aspectMask;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VK_ASSERT(vkCreateImageView(renderContext.m_LogicDevice, &viewInfo, nullptr, &tImageView));
	return tImageView;
}

inline 
VkImageView CreateTextureImageView(VkImage _tImage)
{
	return CreateImageView(_tImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

inline 
VkSampler CreateTextureSampler()
{
	auto renderContext = VKR::render::GetVKContext();
	VkPhysicalDeviceProperties deviceProp;
	VkSamplerCreateInfo samplerInfo{};
	VkSampler TextureSampler;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	vkGetPhysicalDeviceProperties(renderContext.m_GpuInfo.m_Device, &deviceProp);
	samplerInfo.maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;
	VK_ASSERT(vkCreateSampler(renderContext.m_LogicDevice, &samplerInfo, nullptr, &TextureSampler));
	return TextureSampler;
}